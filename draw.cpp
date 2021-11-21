#include <SFML/Graphics.hpp>

#include <vector>
#include <iostream>

#include "const.h"
#include "arena.h"

const float WINDOW_SCALE = 0.064;

sf::Vector2f get_position(float x, float y) {
    return sf::Vector2f(x * WINDOW_SCALE, y * WINDOW_SCALE);
}

struct PodShape : public sf::Drawable {

    Unit *pod;

    sf::CircleShape body;

    std::vector<sf::Vertex> vel;
    std::vector<sf::Vertex> dir;

    void update() {
        
        body.setRadius(POD_RADIUS * WINDOW_SCALE);
        body.setOrigin(POD_RADIUS * WINDOW_SCALE, POD_RADIUS * WINDOW_SCALE);
        body.setPosition(get_position(pod->x, pod->y));
        body.setFillColor(sf::Color::Blue);

        vel = {
            sf::Vertex(get_position(pod->x, pod->y), sf::Color::Green),
            sf::Vertex(get_position(pod->x + pod->vx, 
                                    pod->y + pod->vy), sf::Color::Green)
        };

        dir = {
            sf::Vertex(get_position(pod->x, pod->y), sf::Color::Red),
            sf::Vertex(get_position(pod->x + cos(pod->ang) * POD_RADIUS, 
                                    pod->y + sin(pod->ang) * POD_RADIUS), sf::Color::Red)
        };

    }

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {

        target.draw(body);
        target.draw(&vel[0], vel.size(), sf::Lines);
        target.draw(&dir[0], dir.size(), sf::Lines);

    }

};

struct CheckpointShape : public sf::Drawable {

    Unit *checkpoint;

    sf::CircleShape body;

    void update() {

        body.setRadius(CHECKPOINT_RADIUS * WINDOW_SCALE);
        body.setOrigin(CHECKPOINT_RADIUS * WINDOW_SCALE, CHECKPOINT_RADIUS * WINDOW_SCALE);
        body.setPosition(get_position(checkpoint->x, checkpoint->y));
        body.setFillColor(sf::Color::Black);
        body.setOutlineColor(sf::Color::Yellow);
        body.setOutlineThickness(1.5);

    }

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {

        target.draw(body);

    }

};

Arena arena;

int main() {
    
    arena.init();

    arena.read();

    sf::ContextSettings settings;

    settings.antialiasingLevel = 16.0;

    sf::RenderWindow window(sf::VideoMode(WIDTH * WINDOW_SCALE, HEIGHT * WINDOW_SCALE), "Mad Pod Racing", sf::Style::Close, settings);

    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(10);

    std::vector<PodShape> pods(ALL_PODS);

    for(int i = 0; i < ALL_PODS; i++) {
        pods[i].pod = &arena.pods[i];
    }

    std::vector<CheckpointShape> checkpoints(arena.checkpoints_count);

    for(int i = 0; i < arena.checkpoints_count; i++) {
        checkpoints[i].checkpoint = &arena.checkpoints[i];
    }

    sf::Clock clock;

    while(window.isOpen()) {
        
        for(sf::Event event; window.pollEvent(event); ) {

            switch(event.type) {
                case sf::Event::Closed:
                    window.close();
                    break;
            }

        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            pods[0].pod->ang -= MAX_ROTATION_PER_TURN;
        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            pods[0].pod->ang += MAX_ROTATION_PER_TURN;
        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            pods[0].pod->thrust(650);
        }
        else
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            pods[0].pod->thrust(100);
        }

        arena.tick();

        for(int i = 0; i < 4; i++) {
            std::cerr << arena.pods[i].x << ' ' << arena.pods[i].y << ' ' << arena.pods[i].vx << ' ' << arena.pods[i].vy << ' ' << arena.pods[i].ang << '\n';
        }

        window.clear();

        for(CheckpointShape &cp : checkpoints) {
            cp.update();
            window.draw(cp);
        }

        for(PodShape &pod : pods) {
            pod.update();
            window.draw(pod);
        }

        window.display();

        while(true);

    }

}