#include <logic/game/playing_state.hpp>
#include <logic/game/manager.hpp>
#include <logic/game/bullet.hpp>
#include <logic/game/plane.hpp>
#include <graphics/gl/glfw.hpp>
#include <logic/actor/events.hpp>
#include <logic/game_logic.hpp>
#include <cmath>
#include <fstream>

namespace tung {
namespace state {

class PlaneGenerator: public Process {
private:
    PlayingState& state_;
    milliseconds elapsed_time_;
    milliseconds cycle_;

    const float Tmax_ = 1.0f;
    const float Tmin_ = 0.4f;
    const float after_seconds_ = 60;
    const float value_down_to_ = 0.6f;

    milliseconds get_T() {
        float M = (Tmax_ - Tmin_) / (value_down_to_ - Tmin_);
        float T = Tmin_ + (Tmax_ - Tmin_) 
            * std::pow(M, -elapsed_time_.count() / (1000.0f * after_seconds_));
        return milliseconds(int(T * 1000));
    }

protected:
    void on_init() override {
        Process::on_init();
        elapsed_time_ = 0ms;
        cycle_ = 1000ms;
    }

    void on_update(milliseconds dt) override {
        elapsed_time_ += dt;
        if (elapsed_time_ >= get_T()) {
            elapsed_time_ = 0ms;
            auto plane = std::make_shared<game::Plane>(
                state_.manager_, true
            );
            plane->init();
            plane->start_fly();
            state_.planes_.insert(plane->get_id());
        }
    }

    void on_success() override {
    }

    void on_fail() override {
    }

public:
    PlaneGenerator(PlayingState& state)
    : state_{state} {}
};


PlayingState::PlayingState(Manager& manager)
: GameState(manager) 
{
    background_ = manager_.get_image_factory()
        .new_drawable("assets/playing_background.png", 2);
    plane_generator_ = std::make_shared<PlaneGenerator>(*this);

    show_score_ = std::make_shared<TextView>(
        20, 30, 24, Color::RED, "Score: 0"
    );

    show_high_score_ = std::make_shared<TextView>(
        200, 30, 24, Color::RED, "High Score: 0"
    );

    auto plane_destroy = [this](const IEventData& event_) {
        auto& event = dynamic_cast<const actor::DestroyEvent&>(event_);
        // remove plane
        auto find_it = planes_.find(event.get_id());
        if (find_it != planes_.end()) {
            planes_.erase(find_it);
        }

        // remove bullet
        auto it = bullets_.find(event.get_id());
        if (it != bullets_.end()) {
            bullets_.erase(it);
        }
    };
    plane_destroy_listener_ = plane_destroy;

    auto collide = [this](const IEventData& event_) {
        auto& event = dynamic_cast<const actor::CollideEvent&>(event_);
        auto find_it = planes_.find(event.get_id());
        if (find_it != planes_.end()) {
            auto bullet_it = bullets_.find(event.get_collide_width_id());
            if (bullet_it == bullets_.end())
                return;

            auto ptr = GameLogic::get().get_actor(event.get_id()).lock();
            if (ptr) {
                auto plane = std::dynamic_pointer_cast<game::Plane>(ptr);
                if (plane->is_fighter()) {
                    increase_score(1);
                }
                else {

                }
                plane->explode();
            }
            return;
        }

        auto it = bullets_.find(event.get_id());
        if (it != bullets_.end()) {
            auto ptr = GameLogic::get().get_actor(event.get_id()).lock();
            if (ptr) {
                auto bullet = std::dynamic_pointer_cast<game::Bullet>(ptr);
                bullet->end_fly();
            }
            return;
        }
    };
    collide_listener_ = collide;
}

void PlayingState::increase_score(int value) {
    score_ += value;
    show_score_->set_text("Score: " + std::to_string(score_));
    if (score_ > high_score_) {
        high_score_ = score_;
        show_high_score_->set_text("High Score: " + std::to_string(high_score_));
    }
}

void PlayingState::load_high_score() {
    std::ifstream file{"high_score"};
    if (file) {
        file >> high_score_;
    } 
    else {
        high_score_ = 0;
    }
    show_high_score_->set_text("High Score: " + std::to_string(high_score_));
}

void PlayingState::store_high_score() {
    std::ofstream file{"high_score"};
    file << high_score_;
}

void PlayingState::entry() {
    manager_.get_process_manager().attach_process(plane_generator_);
    manager_.get_event_manager().add_listener(actor::EVENT_DESTROY, 
        plane_destroy_listener_);
    manager_.get_event_manager().add_listener(actor::EVENT_COLLIDE, 
        collide_listener_);

    manager_.get_root()->attach_drawable(background_);
    cannon_ = std::make_shared<game::Cannon>(manager_);
    cannon_->init();

    score_ = 0;
    increase_score(0);
    load_high_score();

    manager_.get_view_root()->add_view(show_score_);
    manager_.get_view_root()->add_view(show_high_score_);
}

void PlayingState::exit() {
    manager_.get_view_root()->remove_view(show_high_score_);
    manager_.get_view_root()->remove_view(show_score_);

    store_high_score();
    // Destroy cannon
    cannon_ = nullptr;

    // Destroy planes
    auto planes = planes_;
    for (auto& plane: planes) {
        actor::DestroyEvent event{plane};
        manager_.get_event_manager().trigger(event);
    }

    // Destroy bullets
    auto bullets = bullets_;
    for (auto& bullet: bullets) {
        actor::DestroyEvent event{bullet};
        manager_.get_event_manager().trigger(event);
    }

    manager_.get_event_manager().remove_listener(actor::EVENT_DESTROY, 
        plane_destroy_listener_);
    manager_.get_event_manager().remove_listener(actor::EVENT_COLLIDE, 
        collide_listener_);

    manager_.get_root()->detach_drawable(background_);
}

bool PlayingState::on_mouse_event(MouseButton button,
    MouseEventType type, float x, float y) 
{
    if (button == MouseButton::LEFT && type == MouseEventType::DOWN) {
        float height = GLFW::get_screen_height();
        float width = GLFW::get_screen_width();
        float ux = x - width / 2;
        float uy = height - y;
        if (ux != 0) {
            float radian = std::atan(uy / ux);
            float degree = radian * 180 / 3.141592654;
            if (degree < 0)
                degree = 180 + degree;
            cannon_->rotate(degree);
            auto bullet_id = cannon_->shot();
            bullets_.insert(bullet_id);
        }
        return true;
    }
    return true;
}

} // namespace state
} // namespace tung