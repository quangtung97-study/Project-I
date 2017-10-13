#ifndef GRAPHICS_ABSTRACT_DRAWABLE_HPP
#define GRAPHICS_ABSTRACT_DRAWABLE_HPP

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <memory>

namespace tung {

struct IShaderProgram;

struct IDrawable {
    virtual const glm::mat4& model_matrix() const = 0;

    virtual void on_draw(IShaderProgram& program) = 0;

    virtual void translate(glm::vec3 vec) = 0;

    virtual void rotate(float angle, glm::vec3 orientation) = 0;

    virtual ~IDrawable() {}
};

typedef std::shared_ptr<IDrawable> IDrawablePtr;

struct IDrawableManager {
    virtual void attach_drawable(const IDrawablePtr& drawable) = 0;

    virtual void detach_drawable(const IDrawablePtr& drawable) = 0;

    virtual ~IDrawableManager() {}
};

} // namespace tung

#endif