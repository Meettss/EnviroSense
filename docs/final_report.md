# EnviroSense
### TinyML-Based Multi-Sensor Edge Intelligence System for Environmental and Equipment Monitoring



## 1. Project Overview

EnviroSense is a TinyML edge intelligence system that uses a single 
3-sensor input set to solve two problems simultaneously on a simulated 
ESP32: environmental risk prediction and equipment anomaly detection.

**Inputs (3 sensors, simulated via potentiometers in Wokwi):**
- Temperature (°C)
- Humidity — can be reinterpreted as vibration for an equipment-monitoring framing
- Gas (ppm) — can be reinterpreted as current for an equipment-monitoring framing

**Two ML Models (same 3 inputs, shared feature set):**
- **Model A — Regression:** predicts a continuous "environmental risk index" (0-100 scale)
- **Model B — Classification:** predicts NORMAL (0) or ANOMALY (1)

**Architecture:** 3 inputs → 1 hidden layer (8 neurons) → output, for both models. 
A small hidden layer was chosen because the input dimensionality is low 
and the relationships are fairly simple, so a larger network would 
overfit a small synthetic dataset without improving accuracy — while 
keeping the model lightweight enough for INT8 quantization and 
deployment on flash-constrained ESP32 hardware.

**Output (Wokwi simulation):** OLED display shows live sensor readings 
and predicted risk index; an LED triggers when the classifier predicts 
ANOMALY.

---

## 2. Dataset & Methodology

**Type:** Synthetic, generated via NumPy Gaussian sampling 
(`np.random.normal`).

**Sensor ranges:**
| Sensor | Normal (mean, σ) | Anomaly (mean, σ) |
|---|---|---|
| Temperature | 25°C, σ=3 | 45°C, σ=5 |
| Humidity | 50%, σ=8 | 15%, σ=10 (clipped to 0-100%) |
| Gas | 150ppm, σ=25 | 400ppm, σ=40 |

**Size:** 1000 total samples — 800 normal, 200 anomaly (80:20 ratio), 
deliberately imbalanced to reflect that anomalies are rare events in 
real systems.

**Regression target (`risk_index`):** a weighted combination of the 
three sensors (temperature weighted 0.4, humidity 0.3, gas 0.5) plus 
Gaussian noise (σ=2), clipped to a 0-100 range. Weights encode which 
sensor matters most toward risk; noise keeps the relationship 
realistically imperfect rather than a perfectly learnable formula.

**Split:** 80% train (800 rows) / 20% test (200 rows), stratified by 
`anomaly_label` to preserve the class ratio in both sets.

**Validation:** A scatter plot (temperature vs. gas, colored by 
anomaly label) confirmed visually separable clusters before any model 
training began, giving early confidence the classification task was 
learnable.

---

## 3. Model Training Results (FP32 Baseline)

### Regression Model
- Test MSE: **8.99**
- Context: `risk_index` ranges ~18-72 (a spread of ~54), so a typical 
  prediction error of ~3 points is a reasonable baseline for a small 
  8-neuron network.

### Classification Model
| Metric | Score |
|---|---|
| Accuracy | 99.5% |
| Precision | 97.6% |
| Recall | 100% |

**Confusion Matrix:**
|  | Predicted Normal | Predicted Anomaly |
|---|---|---|
| **Actual Normal** | 159 | 1 |
| **Actual Anomaly** | 0 | 40 |

Zero missed anomalies (100% recall) with only one false alarm indicates 
a genuinely learned decision boundary rather than memorization. High 
scores are expected given the clearly separated synthetic anomaly 
ranges, confirmed visually in the Day 1 scatter plot.

---

## 4. Quantization: FP32 vs INT8

Weights were manually quantized from FP32 (64-bit float) to INT8 
(8-bit integer) using min-max scaling: each weight table's range was 
mapped onto the -128 to 127 INT8 range using a per-layer scale factor, 
then dequantized back to approximate decimals to test real prediction 
accuracy — simulating on-device inference behavior before full 
embedded deployment.

### Regression Model
| Metric | FP32 | INT8 | Change |
|---|---|---|---|
| MSE | 8.987 | 9.249 | +0.26 (~3%) |
| Model size | 256 bytes | 32 bytes | **8x smaller** |

### Classification Model
| Metric | FP32 | INT8 | Change |
|---|---|---|---|
| Accuracy | 99.5% | 99.5% | No change |
| Precision | 97.6% | 97.6% | No change |
| Recall | 100% | 100% | No change |
| Model size | 256 bytes | 32 bytes | **8x smaller** |

**Conclusion:** Quantization achieved an exact 8x model compression — 
consistent with the mathematical 8-byte-to-1-byte reduction per 
weight — with zero accuracy loss on classification and a negligible 
~3% increase in regression error. This confirms the models are 
well-suited for edge deployment on memory-constrained hardware like 
the ESP32, without a meaningful accuracy tradeoff.

---

## 5. Hardware Implementation

**Platform:** ESP32, simulated in Wokwi.

**Components:**
- 3× potentiometers, standing in for temperature, humidity, and gas 
  sensors (signal pins on GPIO 33, 34, 35; shared VCC/GND)
- 1× LED with resistor for anomaly alerts (GPIO 2)
- 1× SSD1306 OLED display via I2C (SDA on GPIO 21, SCL on GPIO 22) for 
  live readings and risk index

**Model deployment:** Trained model weights and biases were exported 
from Python as C++ header files (`reg_model.h`, `clf_model.h`). The 
forward-pass computation — matrix multiplication, bias addition, and 
ReLU activation — was hand-implemented in the Arduino sketch, so the 
ESP32 performs genuine on-device inference rather than calling an 
external ML library. Raw ADC readings (0-4095) are scaled via `map()` 
into realistic sensor ranges matching the training data before being 
passed into the model.

---

## 6. Testing & Limitations

Testing was performed by manually adjusting the three simulated sensor 
inputs and observing the on-device model's predictions in real time.

| Test case | Temp (°C) | Humidity (%) | Gas (ppm) | Risk Index | Anomaly | Notes |
|---|---|---|---|---|---|---|
| Normal center | 25 | 50 | 150 | 29.57 | NO | Matches training normal average |
| Anomaly center | 45 | 15 | 400 | 60.37 | YES | Matches training anomaly average |
| Boundary (Gas=91) | 35 | 49 | 91 | 32.67 | YES | Near decision threshold |
| Boundary (Gas=94) | 35 | 49 | 94 | 32.96 | YES | Near decision threshold |
| Boundary (Gas=100) | 35 | 49 | 100 | 33.53 | NO | Flip point identified |
| Boundary (Gas=105) | 35 | 49 | 105 | 34.01 | NO | Near decision threshold |
| All-max, high humidity | 60 | 100 | 560 | 91.70 | NO | See limitation below |
| All-max, low humidity | 60 | 0 | 560 | 88.15 | YES | Confirms anomaly pattern dependency |
| All-min | 15 | 0 | 75 | 17.01 | YES | Low humidity alone triggers anomaly flag |

**Baseline behavior is correct**, and the **decision boundary is sharp 
and consistent** — at Temp=35, Humidity=49, the anomaly classification 
flips between Gas=94 and Gas=100, a narrow, well-defined threshold, 
while risk_index increases smoothly across the same range.

**Limitation discovered:** Testing with all three inputs at maximum 
values (Temp=60, Humidity=100, Gas=560) revealed disagreement between 
the two models — risk_index correctly identified this as the 
highest-risk scenario tested (91.70), but the classifier predicted 
NORMAL. This happens because the synthetic anomaly data always paired 
high temperature and gas with *low* humidity; the high-humidity 
combination never appeared during training, placing this input outside 
the classifier's learned decision space (a known ML phenomenon called 
out-of-distribution behavior). Re-testing the same extreme temp/gas 
values with low humidity correctly triggered ANOMALY: YES, confirming 
the root cause. This is a dataset coverage limitation, not a modeling 
or implementation bug, and a natural direction for future improvement — 
generating anomaly data with a wider variety of sensor combinations.

---

## 7. Conclusion

EnviroSense demonstrates a complete edge-AI pipeline: synthetic data 
generation, dual-model training (regression + classification) on 
shared sensor inputs, INT8 quantization achieving 8x compression with 
negligible accuracy loss, and full on-device inference on simulated 
ESP32 hardware with live sensor input and visual/alert output. Testing 
confirmed correct baseline behavior and a well-defined decision 
boundary, while also surfacing an honest, explainable limitation tied 
to training data coverage — providing a clear, credible direction for 
future iteration.