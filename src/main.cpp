// simple CLI scaffold for the pipeline
#include "iostream"
#include <string>
#include "HDF5Reader.cpp/HDF5Reader.h"
#include "PSD.h"
#include "Whiten.h"
#include "BurstDetector.h"
#include <getopt.h>

void print_usage() {
    std::cout << "Usage: ligo_scan --input file.hdf5 --detector H1 --start 0 --end 60 --fs 4096 --out candidates.csv\n";
}

int main(int argc, char** argv) {
    std::string input_file;
    std::string detector = "H1";
    double start_time = 0.0;
    double end_time = 60.0;
    int fs = 4096;
    std::string out_file = "candidates.csv";

    static struct option long_options[] = {
        {"input", required_argument, 0, 'i'},
        {"detector", required_argument, 0, 'd'},
        {"start", required_argument, 0, 's'},
        {"end", required_argument, 0, 'e'},
        {"fs", required_argument, 0, 'f'},
        {"out", required_argument, 0, 'o'},
        {"help", no_argument, 0, 'h'},
        {0,0,0,0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "i:d:s:e:f:o:h", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'i': input_file = optarg; break;
            case 'd': detector = optarg; break;
            case 's': start_time = atof(optarg); break;
            case 'e': end_time = atof(optarg); break;
            case 'f': fs = atoi(optarg); break;
            case 'o': out_file = optarg; break;
            default: print_usage(); return 1;
        }
    }

    if (input_file.empty()) {
        std::cerr << "Error: input file required\n";
        print_usage();
        return 1;
    }

    std::cout << "Reading " << input_file << " for detector " << detector << "\n";
    StrainSeries series;
    try {
        series = HDF5Reader::read_strain(input_file);
    } catch (const std::exception &ex) {
        std::cerr << "Failed to read HDF5: " << ex.what() << "\n";
        return 1;
    }

    // TODO: trim series to requested start/end times (for now assume file contains desired range)
    // Compute PSD (Welch)
    int seg_len = 4 * fs;     // 4-second base segment
    int seg_overlap = seg_len/2;
    std::vector<double> psd = welch_psd(series.data, fs, seg_len, seg_overlap);

    // Whiten entire series in blocks
    std::vector<double> whitened = series.data;
    whiten_blocks(whitened, psd, fs);

    // Run burst detector on whitened series
    std::vector<Candidate> cands = detect_bursts(whitened, series.start_time, fs);

    // Write output CSV
    std::ofstream ofs(out_file);
    ofs << "detector,start_time,peak_time,end_time,snr,central_freq,bandwidth,notes\n";
    for (const auto &c : cands) {
        ofs << detector << "," << c.start_time << "," << c.peak_time << "," << c.end_time << "," << c.snr
            << "," << c.central_freq << "," << c.bandwidth << "," << c.notes << "\n";
    }
    ofs.close();
    std::cout << "Wrote " << cands.size() << " candidates to " << out_file << "\n";
    return 0;
}

