#pragma once

namespace Cool {

static constexpr int64_t ORIGIN_OF_FRAMES = 50'000'000; // Helps with exporting negative frames. Say we already exported frames 0 to 100
                                                        // and then decide that we want to see a bit of the animation before these frames, e.g. exporting frames from -50 to -1.
                                                        // If we name these image files -50.png, -49.png, etc. the alphabetical order will be reversed and most tools won't handle this properly.
                                                        // This is why we don't want negative numbers in our image file names. We therefore have to offset all the frames, in case
                                                        // we might later want to export frames before the current ones.
                                                        // This number, 50'000'000, allows for 115 hours of content at 120 FPS. This should be way more than enough.
                                                        //
                                                        // It is also nice, even if we only have positive frames, that all the frames will have the same number of digits
                                                        // (in the current batch AND the future ones). Because otherwise some tools can struggle ordering them properly.

} // namespace Cool
