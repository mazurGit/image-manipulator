#pragma  once

#include <cstdint>
#include <ostream>


enum class Channel { Red, Green, Blue, Alpha };

class PixelView {
    private:
        uint8_t *data_;
        int channels_;
    public:
    explicit PixelView(uint8_t *data, int channels);
    uint8_t &getColor(Channel channel);
    const uint8_t &getColor(Channel channel) const;

    template <typename Self>
        auto static &getColorImpl(Self &self, Channel channel) {
        {
            int index = static_cast<int>(channel);
            return self.data_[index];
        }
    }

    friend std::ostream& operator<<(
        std::ostream& os,
        const PixelView& pixel
    );
};
