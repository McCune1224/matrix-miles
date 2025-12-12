#include "Transition.h"
#include <cstring>
#include <cmath>

Transition::Transition() 
    : active_(false), type_(TransitionType::None),
      fromPanel_(nullptr), toPanel_(nullptr),
      elapsed_(0), duration_(300) {
}

void Transition::start(TransitionType type, Panel* from, Panel* to,
                       IDisplay* display, uint32_t durationMs) {
    type_ = type;
    fromPanel_ = from;
    toPanel_ = to;
    duration_ = durationMs;
    elapsed_ = 0;
    active_ = true;
    
    // Capture both panels to buffers
    capturePanel(from, display, fromBuffer_);
    capturePanel(to, display, toBuffer_);
}

void Transition::capturePanel(Panel* panel, IDisplay* display, Color* buffer) {
    if (!panel) {
        for (int i = 0; i < MATRIX_WIDTH * MATRIX_HEIGHT; i++) {
            buffer[i] = Color(0, 0, 0);
        }
        return;
    }
    
    // Render panel to display
    panel->render(display);
    
    // Copy framebuffer
    TerminalDisplay* termDisplay = dynamic_cast<TerminalDisplay*>(display);
    if (termDisplay) {
        memcpy(buffer, termDisplay->getFramebuffer(), 
               MATRIX_WIDTH * MATRIX_HEIGHT * sizeof(Color));
    }
}

float Transition::easeInOut(float t) {
    // Smooth ease-in-out curve
    return t < 0.5f 
        ? 2.0f * t * t 
        : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
}

bool Transition::update(uint32_t deltaMs) {
    if (!active_) return true;
    
    elapsed_ += deltaMs;
    
    if (elapsed_ >= duration_) {
        active_ = false;
        return true;
    }
    
    return false;
}

void Transition::render(IDisplay* display) {
    if (!active_) return;
    
    TerminalDisplay* termDisplay = dynamic_cast<TerminalDisplay*>(display);
    if (!termDisplay) return;
    
    float progress = static_cast<float>(elapsed_) / static_cast<float>(duration_);
    progress = easeInOut(progress);
    
    switch (type_) {
        case TransitionType::SlideUp: {
            // Slide from panel moves up, to panel slides in from bottom
            int offset = static_cast<int>((1.0f - progress) * MATRIX_HEIGHT);
            
            for (int y = 0; y < MATRIX_HEIGHT; y++) {
                for (int x = 0; x < MATRIX_WIDTH; x++) {
                    int srcY = y + offset;
                    
                    if (srcY < MATRIX_HEIGHT) {
                        // Show 'to' panel sliding up from bottom
                        termDisplay->setPixelDirect(x, y, 
                            toBuffer_[srcY * MATRIX_WIDTH + x]);
                    } else {
                        // Show 'from' panel being pushed off top
                        int fromY = srcY - MATRIX_HEIGHT;
                        if (fromY >= 0 && fromY < MATRIX_HEIGHT) {
                            termDisplay->setPixelDirect(x, y,
                                fromBuffer_[fromY * MATRIX_WIDTH + x]);
                        }
                    }
                }
            }
            break;
        }
        
        case TransitionType::SlideDown: {
            // Slide from panel moves down, to panel slides in from top
            int offset = static_cast<int>((1.0f - progress) * MATRIX_HEIGHT);
            
            for (int y = 0; y < MATRIX_HEIGHT; y++) {
                for (int x = 0; x < MATRIX_WIDTH; x++) {
                    int srcY = y - (MATRIX_HEIGHT - offset);
                    
                    if (srcY >= 0) {
                        // Show 'to' panel sliding down from top
                        termDisplay->setPixelDirect(x, y,
                            toBuffer_[srcY * MATRIX_WIDTH + x]);
                    } else {
                        // Show 'from' panel being pushed off bottom
                        int fromY = y + offset;
                        if (fromY >= 0 && fromY < MATRIX_HEIGHT) {
                            termDisplay->setPixelDirect(x, y,
                                fromBuffer_[fromY * MATRIX_WIDTH + x]);
                        }
                    }
                }
            }
            break;
        }
        
        case TransitionType::CrossFade: {
            // Blend between panels
            for (int y = 0; y < MATRIX_HEIGHT; y++) {
                for (int x = 0; x < MATRIX_WIDTH; x++) {
                    int idx = y * MATRIX_WIDTH + x;
                    Color from = fromBuffer_[idx];
                    Color to = toBuffer_[idx];
                    
                    Color blended;
                    blended.r = static_cast<uint8_t>(from.r * (1.0f - progress) + to.r * progress);
                    blended.g = static_cast<uint8_t>(from.g * (1.0f - progress) + to.g * progress);
                    blended.b = static_cast<uint8_t>(from.b * (1.0f - progress) + to.b * progress);
                    
                    termDisplay->setPixelDirect(x, y, blended);
                }
            }
            break;
        }
        
        case TransitionType::None:
        default:
            // Just show target panel
            memcpy(termDisplay->getFramebuffer(), toBuffer_,
                   MATRIX_WIDTH * MATRIX_HEIGHT * sizeof(Color));
            break;
    }
}
