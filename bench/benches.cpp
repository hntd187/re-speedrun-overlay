#include "benches.h"

static auto display(std::stringstream& os, std::chrono::nanoseconds ns) {
    const char fill = os.fill();
    os.fill('0');
    const auto h = std::chrono::duration_cast<std::chrono::hours>(ns);
    ns -= h;
    const auto m = std::chrono::duration_cast<std::chrono::minutes>(ns);
    ns -= m;
    const auto s = std::chrono::duration_cast<std::chrono::seconds>(ns);
    ns -= s;
    const auto mill = std::chrono::duration_cast<std::chrono::milliseconds>(ns);
    os << std::setw(2) << h.count() << "h:" << std::setw(2) << m.count() << "m:" << std::setw(2) << s.count() << "s." << mill.count();
    os.fill(fill);
}

static auto display2(std::chrono::nanoseconds ns) {
    const auto h = std::chrono::duration_cast<std::chrono::hours>(ns);
    ns -= h;
    const auto m = std::chrono::duration_cast<std::chrono::minutes>(ns);
    ns -= m;
    const auto s = std::chrono::duration_cast<std::chrono::seconds>(ns);
    ns -= s;
    const auto mill = std::chrono::duration_cast<std::chrono::milliseconds>(ns);
    return std::format("{:%Q}h:{:%Q}m:{:%Q}s.{:%Q}", h, m, s, mill);
    //    return output;
}

static void BM_Time(benchmark::State& state) {
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now);

    for (auto v : state) {
        std::stringstream os{};
        display(os, nanos);
        const auto o = os.str();
        benchmark::DoNotOptimize(o.data());
        benchmark::ClobberMemory();
    }
}

static void BM_Time2(benchmark::State& state) {
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now);

    for (auto _ : state) {
        std::string buf{};
        const std::string o = display2(nanos);
        benchmark::ClobberMemory();
    }
}

static void BM_Time3(benchmark::State& state) {
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now);

    for (auto v : state) {
        std::stringstream os{};
        display(os, nanos);
        const auto o = os.view();
        const auto d = o.data();
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_Time)->Repetitions(15)->ReportAggregatesOnly();
BENCHMARK(BM_Time2)->Repetitions(15)->ReportAggregatesOnly();
BENCHMARK(BM_Time3)->Repetitions(15)->ReportAggregatesOnly();
BENCHMARK_MAIN();