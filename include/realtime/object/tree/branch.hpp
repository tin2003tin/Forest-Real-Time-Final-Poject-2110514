#ifndef BRANCH_HPP
#define BRANCH_HPP

#include <GL/glut.h>
#include <vector>

#include "realtime/core/random.hpp"

namespace realtime {

// ═══════════════════════════════════════════════════════════════
// Branch
// ═══════════════════════════════════════════════════════════════
struct Branch {

    float length;
    float max_length;

    int depth;

    float angle_x;
    float angle_y;
    float angle_z;

    float wind_x = 0.0f;
    float wind_y = 0.0f;

    float wind_vel_x = 0.0f;
    float wind_vel_y = 0.0f;

    float noise_seed;

    std::vector<Branch> children;

    // ───────────────────────────────────────────────────────────
    // Leaf quad data
    // ───────────────────────────────────────────────────────────
    struct LeafQuad {

        float ox, oy, oz;

        float nx, ny, nz;

        float ax, ay, az;

        float bx, by, bz;

        float size;

        float phase;

        float cr, cg, cb;
    };

    std::vector<LeafQuad> leaf_quads;

    bool leaves_built = false;

    // ───────────────────────────────────────────────────────────
    // Display list cache
    //
    // dl_id    : OpenGL display list name (0 = not yet compiled)
    // dl_dirty : true whenever geometry has changed and the list
    //            must be recompiled before the next draw call.
    //
    // Lifecycle:
    //   grown = false  →  dl_id == 0, dl_dirty == true
    //   first draw after fully grown  →  compile list, dl_dirty=false
    //   branch spawns new children    →  dl_dirty=true on this branch
    //   tree deleted / moved          →  free_display_list() called
    // ───────────────────────────────────────────────────────────
    GLuint dl_id    = 0;
    bool   dl_dirty = true;

    // Release the GL list if one was allocated. Safe to call
    // multiple times (guards on dl_id != 0).
    void free_display_list() {
        if (dl_id != 0) {
            glDeleteLists(dl_id, 1);
            dl_id = 0;
        }
        dl_dirty = true;
        for (auto& c : children) c.free_display_list();
    }

    // Mark this branch and every ancestor as dirty so the next
    // draw pass recompiles their lists. Call this whenever
    // length changes or children are added.
    void invalidate() {
        dl_dirty = true;
        // Children are re-compiled independently; the parent list
        // only contains the cylinder/cap for THIS segment, so we
        // only need to mark the direct branch dirty, not recurse.
    }

    // ───────────────────────────────────────────────────────────
    // Constructor
    // ───────────────────────────────────────────────────────────
    Branch(
        float len,
        int d,
        float seed = 0
    )
        :
        length(0),
        max_length(len),
        depth(d),

        angle_x(randf(-38, 38)),
        angle_y(randf(-38, 38)),
        angle_z(randf(-20, 20)),

        noise_seed(seed + randf(0, 100))
    {
    }

    // Prevent accidental copies — display list IDs are not
    // copyable (they'd be double-freed). Only moves are allowed.
    Branch(const Branch&) = delete;
    Branch& operator=(const Branch&) = delete;

    Branch(Branch&& o) noexcept
        : length(o.length), max_length(o.max_length),
          depth(o.depth),
          angle_x(o.angle_x), angle_y(o.angle_y), angle_z(o.angle_z),
          noise_seed(o.noise_seed),
          children(std::move(o.children)),
          leaf_quads(std::move(o.leaf_quads)),
          leaves_built(o.leaves_built),
          dl_id(o.dl_id), dl_dirty(o.dl_dirty)
    {
        o.dl_id    = 0;   // moved-from: don't free our list
        o.dl_dirty = true;
    }

    Branch& operator=(Branch&& o) noexcept {
        if (this != &o) {
            free_display_list();
            length       = o.length;
            max_length   = o.max_length;
            depth        = o.depth;
            angle_x      = o.angle_x;
            angle_y      = o.angle_y;
            angle_z      = o.angle_z;
            noise_seed   = o.noise_seed;
            children     = std::move(o.children);
            leaf_quads   = std::move(o.leaf_quads);
            leaves_built = o.leaves_built;
            dl_id        = o.dl_id;
            dl_dirty     = o.dl_dirty;
            o.dl_id      = 0;
            o.dl_dirty   = true;
        }
        return *this;
    }

    ~Branch() {
        // Note: glDeleteLists must only be called while an OpenGL
        // context is current. Tree::~Tree() calls
        // root.free_display_list() before the destructor chain
        // reaches here, so this is just a safety net.
        if (dl_id != 0) {
            glDeleteLists(dl_id, 1);
            dl_id = 0;
        }
    }
};

} // namespace realtime

#endif