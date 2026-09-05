# EnviroSense — Project Architecture

## Overview
EnviroSense is a TinyML edge intelligence system that uses a single 3-sensor 
input set to solve two problems simultaneously on a simulated ESP32: 
environmental risk prediction and equipment anomaly detection.

## Inputs (3 sensors, simulated via potentiometers in Wokwi)
- Temperature (°C)
- Humidity / vibration-proxy (context-dependent: humidity for environmental 
  framing, vibration for equipment framing — same feature slot, dataset is synthetic)
- Gas / current-proxy (ppm or current, same dual-framing logic)

## Two ML Models (same 3 inputs, shared feature set)

### Model A — Regression
- Task: predict a continuous "environmental risk index" (0–100 scale)
- Architecture: 3 input → 1 hidden layer (8 neurons) → 1 output
- Metric: MSE (Mean Squared Error)

### Model B — Classification
- Task: predict NORMAL (0) vs ANOMALY (1)
- Architecture: 3 input → 1 hidden layer (8 neurons) → 2-class output
- Metrics: Accuracy, Precision, Recall, Confusion Matrix

## Why 8 neurons, 1 hidden layer
Input dimensionality is low (3 features) with a fairly simple/near-linear 
relationship. A small hidden layer avoids overfitting on a small synthetic 
dataset and keeps both models lightweight enough for INT8 quantization and 
deployment on flash-constrained ESP32 hardware.

## Output (Wokwi simulation)
- OLED display: live sensor readings + predicted risk index (from Model A)
- LED/buzzer: triggers when Model B predicts ANOMALY

## Quantization
Both models trained in FP32, then converted to INT8. Comparison table 
(size, latency, accuracy/MSE delta) is a core result of the project, 
demonstrating edge-deployment optimization.

## Data
Synthetic, generated via NumPy Gaussian sampling — disclosed explicitly 
due to 1-week timeline and no physical hardware access.