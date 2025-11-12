#include "BurstDetector.h"
#include <vector>
#include <cmath>

// Very simple sliding-window RMS detector (skeleton)
std::vector<Candidate> detect_bursts(const std::vector<double>& whitened, double start_time, int fs) {
    std::vector<Candidate> results;
    int n = (int)whitened.size();
    int win_samples = std::max(1, fs/10); // 0.1s window
    int hop = win_samples / 2;
    double mean = 0.0, var = 0.0;
    // Estimate global mean/var (white data should be ~0 mean, unit variance after whitening)
    for (int i=0;i<n;i++) mean += whitened[i];
    mean /= n;
    for (int i=0;i<n;i++) var += (whitened[i]-mean)*(whitened[i]-mean);
    var /= n;
    double stddev = std::sqrt(var);

    double threshold = 5.0 * stddev; // arbitrary threshold (tune later)

    for (int s=0; s+win_samples <= n; s += hop) {
        double sum2 = 0.0;
        for (int i=0;i<win_samples;i++) sum2 += whitened[s+i]*whitened[s+i];
        double rms = std::sqrt(sum2 / win_samples);
        if (rms > threshold) {
            Candidate c;
            c.start_time = start_time + (double)s / fs;
            c.peak_time = start_time + (double)(s + win_samples/2) / fs;
            c.end_time = start_time + (double)(s + win_samples) / fs;
            c.snr = rms;
            c.central_freq = 0.0; // placeholder: compute via short-FFT if desired
            c.bandwidth = 0.0;
            c.notes = "excess_power_rms";
            results.push_back(c);
            // skip some samples to avoid dense detections
            s += win_samples;
        }
    }
    return results;
}
