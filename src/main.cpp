#include "ams2_telemetry.h"
#include <chrono>
#include <deque>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/canvas.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <iostream>
#include <stop_token>
#include <thread>

using namespace std::chrono_literals;

static constexpr int CANVAS_WIDTH = 200;
static constexpr int CANVAS_HEIGHT = 100;
static constexpr int HISTORY_SIZE = CANVAS_WIDTH;

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

  std::deque<float> throttle_history(HISTORY_SIZE, 0.0f);
  std::deque<float> brake_history(HISTORY_SIZE, 0.0f);
  std::deque<float> steering_history(HISTORY_SIZE, 0.5f);

  auto renderer = ftxui::Renderer([&] {
    ams2_telemetry tele;
    read_ams2_telemetry(pid, &tele, remote_addr);

    throttle_history.push_front(tele.mUnfilteredThrottle);
    throttle_history.pop_back();
    brake_history.push_front(tele.mUnfilteredBrake);
    brake_history.pop_back();
    steering_history.push_front((tele.mUnfilteredSteering + 1.0f) / 2.0f);
    steering_history.pop_back();

    auto c = ftxui::Canvas(CANVAS_WIDTH, CANVAS_HEIGHT);

    for (int x = 0; x < HISTORY_SIZE - 1; x++) {
      int y0 = (int)((1.0f - throttle_history[x]) * (CANVAS_HEIGHT - 1));
      int y1 = (int)((1.0f - throttle_history[x + 1]) * (CANVAS_HEIGHT - 1));
      c.DrawPointLine(x, y0, x + 1, y1, ftxui::Color::Green);

      y0 = (int)((1.0f - brake_history[x]) * (CANVAS_HEIGHT - 1));
      y1 = (int)((1.0f - brake_history[x + 1]) * (CANVAS_HEIGHT - 1));
      c.DrawPointLine(x, y0, x + 1, y1, ftxui::Color::Red);

      y0 = (int)((1.0f - steering_history[x]) * (CANVAS_HEIGHT - 1));
      y1 = (int)((1.0f - steering_history[x + 1]) * (CANVAS_HEIGHT - 1));
      c.DrawPointLine(x, y0, x + 1, y1, ftxui::Color::Blue);
    }

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
        canvas(std::move(c)) | border,
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
