# Laboratory Work 4 — Data Visualization 2

## Description
Interactive program for plotting a harmonic function with noise:
y(t) = A * sin(wt + φ)

The application allows:
- changing amplitude, frequency, phase
- changing noise mean and covariance
- showing/hiding noisy signal
- filtering noisy harmonic
- resetting parameters

## Files
- `task.py` — main program
- `requirements.txt` — dependencies

## Run
```bash
py -m venv venv
source venv/Scripts/activate
pip install -r requirements.txt
python task.py