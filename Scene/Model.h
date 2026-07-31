#pragma once

/**
 * Model.h
 *
 * Abstract base class for all renderable objects.
 * Platform-agnostic – shared by Android, Desktop, and WebGL builds.
 */


class Model {
public:
    Model() {}
    virtual ~Model() {}

    virtual void InitModel()          = 0;
    virtual void Render()             = 0;
    virtual void Resize(int w, int h) {}

};
