#pragma once
// meridian-terminal / core / vt / graphics.hpp
//
// Complete high-performance Sixel raster and Kitty Graphics Protocol engine.
// Implements chunked transfers, z-index layering, cell anchoring, and DEC Sixel decoding.

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace meridian::vt {

enum class ImageFormat {
    Rgba32,
    Rgb24,
    Png,
    Sixel
};

enum class TransmissionType {
    Direct,     // 'd'
    File,       // 'f'
    TempFile,   // 't'
    SharedMem   // 's'
};

enum class KittyAction {
    Transmit,            // 't'
    TransmitAndDisplay,  // 'T'
    Query,               // 'q'
    Put,                 // 'p'
    Delete               // 'd'
};

struct GraphicImage {
    uint32_t id = 0;
    int width = 0;
    int height = 0;
    ImageFormat format = ImageFormat::Rgba32;
    std::vector<uint8_t> rgba_data; // 32-bit RGBA pixel buffer (w * h * 4 bytes)
};

struct ImagePlacement {
    uint32_t image_id = 0;
    uint32_t placement_id = 0;
    int row = 0;
    int col = 0;
    int cols_spanned = 1;
    int rows_spanned = 1;
    int x_offset = 0;
    int y_offset = 0;
    int z_index = 0;
    bool cursor_relative = true;
};

class GraphicsEngine {
public:
    using ResponseCallback = std::function<void(const std::string&)>;

    GraphicsEngine();

    // Kitty Graphics Protocol APC parser: "\033_Ga=T,f=32,s=100,v=100;...base64...\033\\"
    bool handle_kitty_graphics(std::string_view payload, int cursor_row = 0, int cursor_col = 0);

    // Complete DEC Sixel raster graphics decoder: "\033Pq...data...\033\\"
    bool handle_sixel(std::string_view sixel_stream, int cursor_row = 0, int cursor_col = 0,
                      int cell_width = 9, int cell_height = 18);

    const GraphicImage* find_image(uint32_t id) const;
    const std::vector<ImagePlacement>& placements() const { return placements_; }
    void clear_placements() { placements_.clear(); }
    void clear_all();

    void set_response_callback(ResponseCallback cb) { response_cb_ = std::move(cb); }

    // Base64 helper
    static std::vector<uint8_t> decode_base64(std::string_view in);

private:
    uint32_t next_image_id_ = 1;
    uint32_t next_placement_id_ = 1;
    std::unordered_map<uint32_t, GraphicImage> images_;
    std::vector<ImagePlacement> placements_;

    // Chunk reassembly buffer for multi-packet transmissions (m=1/0)
    struct ChunkSession {
        uint32_t image_id = 0;
        ImageFormat format = ImageFormat::Rgba32;
        TransmissionType trans_type = TransmissionType::Direct;
        int width = 0;
        int height = 0;
        int cols = 0;
        int rows = 0;
        int z_index = 0;
        bool quiet = false;
        KittyAction action = KittyAction::TransmitAndDisplay;
        std::string accumulated_payload;
    };
    std::unordered_map<uint32_t, ChunkSession> chunk_sessions_;
    ChunkSession current_anonymous_session_;

    ResponseCallback response_cb_;

    void process_kitty_command(const ChunkSession& session, int cursor_row, int cursor_col);
    void send_kitty_response(uint32_t image_id, uint32_t placement_id, const std::string& msg);
};

} // namespace meridian::vt
