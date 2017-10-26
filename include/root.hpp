#ifndef GAME_ROOT_HPP
#define GAME_ROOT_HPP

#include <memory>
#include <graphics/gl/glfw.hpp>
#include <graphics/abstract/shader_program.hpp>
#include <graphics/abstract/object.hpp>
#include <graphics/abstract/image.hpp>
#include <graphics/abstract/texture.hpp>
#include <sound/abstract/sound.hpp>

namespace tung {

class Root {
private:
    std::unique_ptr<GLFW> glfw_;
    std::unique_ptr<IShaderProgram> program_;
    std::unique_ptr<IImageLoader> image_loader_;
    std::unique_ptr<ITextureFactory> texture_factory_;
    std::unique_ptr<IVertexObjectBuilder> object_builder_;

    // Sound
    std::unique_ptr<ISoundManager> sound_manager_;

public:
    Root();

    void run();

    ~Root();
};

} // namespace tung

#endif