# EnviroSense

**TinyML-based edge intelligence system for environmental and equipment monitoring.**

A dual-model system that predicts environmental risk and detects equipment 
anomalies from the same three sensor inputs — trained in Python, compressed 
via quantization, and deployed as hand-written C++ inference on a simulated 
ESP32 (Wokwi).

---

## Results

| | |
|---|---|
| Model compression | **8x** (FP32 → INT8, manual quantization) |
| Classification accuracy | **98%** (recall 92.5%, across two anomaly patterns) |
| Regression error | MSE 6.99 |
| Deployment | Real on-device inference, no external ML library |

A limitation was discovered during testing, fixed by improving the dataset, 
and confirmed live on hardware with a v1/v2 toggle. Full story in the report below.

---

## How it works

**Inputs:** temperature, humidity, gas — 3 simulated sensors (potentiometers)

**Two models, shared inputs:**
- Regression → continuous risk index (0–100)
- Classification → NORMAL / ANOMALY

**Architecture:** 3 → 8 hidden (ReLU) → output, for both models

**Output:** OLED display for live readings + risk index, LED alert on anomaly

---

## Repo structure
data/ synthetic datasets (v1, v2)
models/ trained models, quantized weights, C++ headers
notebooks/ Colab notebooks — data generation, training, quantization
wokwi/ ESP32 sketch, wiring, and standalone HTML dashboard
docs/ architecture, testing results, full report

---

## Read more

- **[Full report](docs/final_report.md)** — dataset, training, quantization, hardware, testing, and the v1→v2 fix
- **[Live demo](wokwi/dashboard.html)** — open in any browser

---

## Built with

Python · scikit-learn · NumPy · pandas · Wokwi · C++/Arduino · HTML/JS