# EnviroSense — Testing Results

## Methodology
Testing was performed by manually adjusting the three simulated sensor 
inputs (potentiometers standing in for temperature, humidity, and gas) 
in the Wokwi simulation, and observing the on-device model's risk_index 
and anomaly predictions in real time via Serial Monitor and OLED display.

Five categories of tests were run: normal-center baseline, anomaly-center 
baseline, decision boundary mapping, extreme/out-of-range values, and a 
targeted follow-up to investigate an unexpected result.

## Results

| Test case | Temp (°C) | Humidity (%) | Gas (ppm) | Risk Index | Anomaly | Notes |
|---|---|---|---|---|---|---|
| Normal center | 25 | 50 | 150 | 29.57 | NO | Matches training data normal average |
| Anomaly center | 45 | 15 | 400 | 60.37 | YES | Matches training data anomaly average |
| Boundary (Gas=91) | 35 | 49 | 91 | 32.67 | YES | Near decision threshold |
| Boundary (Gas=94) | 35 | 49 | 94 | 32.96 | YES | Near decision threshold |
| Boundary (Gas=100) | 35 | 49 | 100 | 33.53 | NO | Flip point identified |
| Boundary (Gas=105) | 35 | 49 | 105 | 34.01 | NO | Near decision threshold |
| All-max, high humidity | 60 | 100 | 560 | 91.70 | NO | See Limitation below |
| All-max, low humidity | 60 | 0 | 560 | 88.15 | YES | Confirms anomaly pattern dependency |
| All-min | 15 | 0 | 75 | 17.01 | YES | Low humidity alone triggers anomaly flag |

## Key Findings

**Baseline behavior is correct.** Inputs matching the training data's 
normal and anomaly centers produce confident, correctly-classified 
predictions with risk_index values consistent with training data ranges.

**Decision boundary is sharp and consistent.** At Temp=35, Humidity=49, 
the anomaly classification flips between Gas=94 and Gas=100 — a narrow, 
well-defined threshold rather than erratic behavior. The risk_index 
increases smoothly and monotonically across this range (32.67 → 34.01), 
confirming the regression and classification models behave independently 
and consistently.

## Limitation Discovered

Testing with all three inputs at maximum values (Temp=60, Humidity=100, 
Gas=560) revealed disagreement between the two models: risk_index 
correctly identified this as the highest-risk scenario tested (91.70), 
but the classifier predicted NORMAL rather than ANOMALY.

**Root cause:** The synthetic anomaly data generation always paired 
high temperature and high gas with LOW humidity (mean ~15%). The 
combination of high temp, high gas, AND high humidity never appeared 
in the training set, placing this test case outside the classifier's 
learned decision space — a known machine learning limitation called 
out-of-distribution behavior.

This was confirmed by re-testing the same extreme temp/gas values with 
low humidity (Temp=60, Humidity=0, Gas=560), which correctly triggered 
ANOMALY: YES (risk 88.15) — matching the pattern the classifier was 
actually trained on.

**Implication:** This is a dataset coverage limitation, not a modeling 
or implementation bug. A future iteration could address this by 
generating anomaly training data with a wider variety of sensor 
combinations (e.g., high-humidity anomalies representing a different 
failure mode, such as condensation or flooding) rather than a single 
fixed anomaly pattern.