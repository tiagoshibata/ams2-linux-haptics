#include "ams2_telemetry.h"
#include <chrono>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <iostream>
#include <stop_token>
#include <thread>

using namespace std::chrono_literals;

int main() {
  int pid;
  while (!(pid = get_ams2_pid())) {
    std::cout << "No AMS2 process found. Retrying...\n";
    std::this_thread::sleep_for(5s);
  }
  std::cout << "Found AMS2 running with PID " << pid << '\n';

  void *remote_addr;
  while (!(remote_addr = get_ams2_telemetry_address(pid))) {
    std::cout << "AMS2 telemetry shared memory not found. Make sure AMS2 is fully initialized, and the \"shared "
                 "memory\" setting is enabled. Retrying...\n";
    std::this_thread::sleep_for(5s);
  }

  auto renderer = ftxui::Renderer([&] {
    ams2_telemetry tele;
    read_ams2_telemetry(pid, &tele, remote_addr);

    using namespace ftxui;
    return vbox({
        text("AMS2 Telemetry") | bold,
        separator(),
        hbox({
            text("Throttle: "),
            gauge(tele.mUnfilteredThrottle) | color(Color::Green),
            text(" " + std::to_string((int)(tele.mUnfilteredThrottle * 100)) + "%"),
        }),
        hbox({
            text("Brake:    "),
            gauge(tele.mUnfilteredBrake) | color(Color::Red),
            text(" " + std::to_string((int)(tele.mUnfilteredBrake * 100)) + "%"),
        }),
        hbox({
            text("Steering: "),
            gauge((tele.mUnfilteredSteering + 1.0f) / 2.0f) | color(Color::Blue),
            text(" " + std::to_string((int)((tele.mUnfilteredSteering + 1.0f) / 2.0f * 100)) + "%"),
        }),
    });
  });

  auto screen = ftxui::ScreenInteractive::Fullscreen();
  std::jthread t([&screen](std::stop_token st) {
    while (!st.stop_requested()) {
      std::this_thread::sleep_for(20ms);
      screen.Post(ftxui::Event::Custom);
    }
  });

  screen.Loop(renderer);

  return 0;
}
