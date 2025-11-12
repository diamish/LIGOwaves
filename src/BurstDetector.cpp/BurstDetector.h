#pragma once
#include <string>
#include <vector>

struct Candidate {
    double start_time = 0.0;
    double peak_time = 0.0;
    double end_time = 0.0;
    double snr = 0.0;
    double central_freq = 0.0;
    double bandwidth = 0.0;
    std::string notes;
};

// Basic excess-power burst detector
std::vector<Candidate> detect_bursts(const std::vector<double>& whitened, double start_time, int fs);


