#pragma once

class MainLayer : public zth::Layer
{
public:
    MainLayer() = default;
    ZTH_NO_COPY_NO_MOVE(MainLayer)
    ~MainLayer() override = default;

public:
    auto on_event(const zth::Event& event) -> void override;
};
