#include "ams2_telemetry.h"
#include <deque>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/canvas.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <stop_token>
#include <thread>

using namespace std::chrono_literals;

constexpr int CANVAS_WIDTH = 200;
constexpr int CANVAS_HEIGHT = 100;
constexpr int HISTORY_SIZE = CANVAS_WIDTH;
constexpr float TYRE_DIAMETER = 0.584; // Ginetta G40

constexpr float compute_slip(float tyre_rps, float x_vel) {
  if (std::abs(x_vel) < 0.01) {
    return 0;
  }
  auto tyre_surface_speed = TYRE_DIAMETER / 2 * tyre_rps;
  auto slip = tyre_surface_speed / x_vel;
  return std::min(std::max(slip, -1.f), 1.f);
}

int main() {
  auto pid = wait_for_ams2_pid();
  const auto *remote_addr = wait_for_ams2_telemetry_address(pid);

  std::deque<float> slip_fl(HISTORY_SIZE, 0.0f);
  std::deque<float> slip_fr(HISTORY_SIZE, 0.0f);
  std::deque<float> slip_rl(HISTORY_SIZE, 0.0f);
  std::deque<float> slip_rr(HISTORY_SIZE, 0.0f);

  auto renderer = ftxui::Renderer([&] {
    ams2_telemetry tele;
    read_ams2_telemetry(pid, &tele, remote_addr);

    auto x_vel = tele.mLocalVelocity[2];
    auto fl = compute_slip(tele.mTyreRPS[0], x_vel);
    auto fr = compute_slip(tele.mTyreRPS[1], x_vel);
    auto rl = compute_slip(tele.mTyreRPS[2], x_vel);
    auto rr = compute_slip(tele.mTyreRPS[3], x_vel);
    slip_fl.push_front(fl);
    slip_fr.push_front(fr);
    slip_rl.push_front(rl);
    slip_rr.push_front(rr);

    slip_fl.pop_back();
    slip_fr.pop_back();
    slip_rl.pop_back();
    slip_rr.pop_back();

    auto c = ftxui::Canvas(CANVAS_WIDTH, CANVAS_HEIGHT);
    for (int x = 0; x < HISTORY_SIZE - 1; x++) {
      int y0 = (int)((.5f - slip_fl[x] / 2) * (CANVAS_HEIGHT - 1));
      int y1 = (int)((.5f - slip_fl[x + 1] / 2) * (CANVAS_HEIGHT - 1));
      c.DrawPointLine(x, y0, x + 1, y1, ftxui::Color::Green);

      y0 = (int)((.5f - slip_fr[x] / 2) * (CANVAS_HEIGHT - 1));
      y1 = (int)((.5f - slip_fr[x + 1] / 2) * (CANVAS_HEIGHT - 1));
      c.DrawPointLine(x, y0, x + 1, y1, ftxui::Color::Red);

      y0 = (int)((.5f - slip_rl[x] / 2) * (CANVAS_HEIGHT - 1));
      y1 = (int)((.5f - slip_rl[x + 1] / 2) * (CANVAS_HEIGHT - 1));
      c.DrawPointLine(x, y0, x + 1, y1, ftxui::Color::Blue);

      y0 = (int)((.5f - slip_rr[x] / 2) * (CANVAS_HEIGHT - 1));
      y1 = (int)((.5f - slip_rr[x + 1] / 2) * (CANVAS_HEIGHT - 1));
      c.DrawPointLine(x, y0, x + 1, y1, ftxui::Color::Yellow);
    }

    using namespace ftxui;
    return vbox({
        text(std::format("Speed: {}m/s Tyre: {}RPS = {}m/s", x_vel, tele.mTyreRPS[0], TYRE_DIAMETER / 2 * tele.mTyreRPS[0])) | bold,
        separator(),
        hbox({
            text("FL: "),
            gauge(.5f + fl / 2) | color(Color::Green),
            text(" " + std::to_string((int)(fl * 100)) + "%"),
        }),
        hbox({
            text("FR: "),
            gauge(.5f + fr / 2) | color(Color::Red),
            text(" " + std::to_string((int)(fr * 100)) + "%"),
        }),
        hbox({
            text("RL: "),
            gauge(.5f + rl / 2) | color(Color::Blue),
            text(" " + std::to_string((int)(rl * 100)) + "%"),
        }),
        hbox({
            text("RR: "),
            gauge(.5f + rr / 2) | color(Color::Yellow),
            text(" " + std::to_string((int)(rr * 100)) + "%"),
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
