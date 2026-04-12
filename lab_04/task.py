import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider, Button, CheckButtons
from scipy import signal



INITIALS = {
    "amplitude": 1.0,
    "frequency": 1.5,
    "phase": 0.0,
    "noise_mean": 0.0,
    "noise_covariance": 0.12,
    "cutoff": 3.0,
    "show_noise": True,
}

T = np.linspace(0, 10, 1000)

state = {
    "amplitude": INITIALS["amplitude"],
    "frequency": INITIALS["frequency"],
    "phase": INITIALS["phase"],
    "noise_mean": INITIALS["noise_mean"],
    "noise_covariance": INITIALS["noise_covariance"],
    "cutoff": INITIALS["cutoff"],
    "show_noise": INITIALS["show_noise"],
    "noise": None,
}



def harmonic_with_noise(amplitude, frequency, phase, noise_mean, noise_covariance, show_noise=True):
    clean = amplitude * np.sin(frequency * T + phase)

    noisy = clean + state["noise"]

    if show_noise:
        return clean, noisy
    return clean, clean


def generate_noise(mean, covariance):
    std = np.sqrt(max(covariance, 1e-8))
    return np.random.normal(loc=mean, scale=std, size=T.shape)


def lowpass_filter(data, cutoff):
    sample_rate = len(T) / (T[-1] - T[0])
    nyquist = 0.5 * sample_rate
    normalized_cutoff = min(max(cutoff / nyquist, 1e-4), 0.99)

    b, a = signal.butter(4, normalized_cutoff, btype="low")
    return signal.filtfilt(b, a, data)


def update_plot(_=None):
    clean, noisy_or_clean = harmonic_with_noise(
        state["amplitude"],
        state["frequency"],
        state["phase"],
        state["noise_mean"],
        state["noise_covariance"],
        state["show_noise"],
    )

    filtered = lowpass_filter(clean + state["noise"], state["cutoff"])

    line_clean.set_ydata(clean)

    if state["show_noise"]:
        line_noisy.set_ydata(noisy_or_clean)
        line_noisy.set_visible(True)
    else:
        line_noisy.set_visible(False)

    line_filtered.set_ydata(filtered)

    fig.canvas.draw_idle()


def on_slider_change(_):
    old_noise_mean = state["noise_mean"]
    old_noise_cov = state["noise_covariance"]

    state["amplitude"] = s_amp.val
    state["frequency"] = s_freq.val
    state["phase"] = s_phase.val
    state["noise_mean"] = s_noise_mean.val
    state["noise_covariance"] = s_noise_cov.val
    state["cutoff"] = s_cutoff.val

    if (
        not np.isclose(old_noise_mean, state["noise_mean"])
        or not np.isclose(old_noise_cov, state["noise_covariance"])
    ):
        state["noise"] = generate_noise(state["noise_mean"], state["noise_covariance"])

    update_plot()


def on_checkbox_click(_):
    state["show_noise"] = not state["show_noise"]
    update_plot()


def on_reset(_):
    s_amp.reset()
    s_freq.reset()
    s_phase.reset()
    s_noise_mean.reset()
    s_noise_cov.reset()
    s_cutoff.reset()

    state["show_noise"] = INITIALS["show_noise"]
    state["noise"] = generate_noise(INITIALS["noise_mean"], INITIALS["noise_covariance"])

    current_status = check.get_status()[0]
    if current_status != INITIALS["show_noise"]:
        check.set_active(0)

    update_plot()


state["noise"] = generate_noise(state["noise_mean"], state["noise_covariance"])

fig, ax = plt.subplots(figsize=(12, 7))
plt.subplots_adjust(left=0.10, bottom=0.42, right=0.92)

clean0, noisy0 = harmonic_with_noise(
    state["amplitude"],
    state["frequency"],
    state["phase"],
    state["noise_mean"],
    state["noise_covariance"],
    state["show_noise"],
)
filtered0 = lowpass_filter(clean0 + state["noise"], state["cutoff"])

line_clean, = ax.plot(T, clean0, color="blue", linewidth=2, label="Clean harmonic")
line_noisy, = ax.plot(T, noisy0, color="orange", linewidth=1.5, label="Noisy harmonic")
line_filtered, = ax.plot(T, filtered0, color="red", linewidth=2, label="Filtered harmonic")

ax.set_title("Laboratory Work 4 — Harmonic with Noise and Filtering")
ax.set_xlabel("t")
ax.set_ylabel("y(t)")
ax.grid(True)
ax.legend()

ax_amp = plt.axes([0.18, 0.30, 0.55, 0.03])
ax_freq = plt.axes([0.18, 0.25, 0.55, 0.03])
ax_phase = plt.axes([0.18, 0.20, 0.55, 0.03])
ax_noise_mean = plt.axes([0.18, 0.15, 0.55, 0.03])
ax_noise_cov = plt.axes([0.18, 0.10, 0.55, 0.03])
ax_cutoff = plt.axes([0.18, 0.05, 0.55, 0.03])

s_amp = Slider(ax_amp, "Amplitude", 0.1, 3.0, valinit=INITIALS["amplitude"])
s_freq = Slider(ax_freq, "Frequency", 0.1, 5.0, valinit=INITIALS["frequency"])
s_phase = Slider(ax_phase, "Phase", 0.0, 2 * np.pi, valinit=INITIALS["phase"])
s_noise_mean = Slider(ax_noise_mean, "Noise Mean", -1.0, 1.0, valinit=INITIALS["noise_mean"])
s_noise_cov = Slider(ax_noise_cov, "Noise Covariance", 0.001, 1.0, valinit=INITIALS["noise_covariance"])
s_cutoff = Slider(ax_cutoff, "Cutoff Frequency", 0.1, 10.0, valinit=INITIALS["cutoff"])

for slider in [s_amp, s_freq, s_phase, s_noise_mean, s_noise_cov, s_cutoff]:
    slider.on_changed(on_slider_change)

ax_check = plt.axes([0.80, 0.06, 0.12, 0.08])
check = CheckButtons(ax_check, ["Show Noise"], [INITIALS["show_noise"]])
check.on_clicked(on_checkbox_click)

ax_button = plt.axes([0.10, 0.05, 0.08, 0.04])
btn_reset = Button(ax_button, "Reset")
btn_reset.on_clicked(on_reset)

fig.text(
    0.10,
    0.36,
    "Instructions:\n"
    "1. Use sliders to change harmonic and noise parameters.\n"
    "2. 'Show Noise' toggles noisy signal visibility.\n"
    "3. Reset returns all parameters to initial values.\n"
    "4. Filtered signal should be close to the clean harmonic.",
    fontsize=10,
    va="top"
)

plt.show()