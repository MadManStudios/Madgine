#include "../metalib.h"

#include "atlas2.h"

#include "common.h"

namespace Engine {

std::tuple<float, int, bool> Atlas2::getScore(Bin &bin, const Vector2i &size, bool allowFlip)
{
    float score = 0;
    int cornerIndex;
    bool flipped;

    for (int index = 0; index < bin.mCorners.size(); ++index) {
        const Vector2i &pos = bin.mCorners[index];

        int rightEnd = index > 0 ? bin.mCorners[index - 1].x : mBinSize.x;
        int bottomEnd = (index < bin.mCorners.size() - 1) ? bin.mCorners[index + 1].y : mBinSize.y;

        Vector2i cornerBounds { rightEnd - pos.x, bottomEnd - pos.y };

        if (pos.x + size.x <= mBinSize.x && pos.y + size.y <= mBinSize.y) {
            float score_x = float(cornerBounds.x) / size.x;
            if (score_x > 1.0f)
                score_x = 1.0f;
            float score_y = float(cornerBounds.y) / size.y;
            if (score_y > 1.0f)
                score_y = 1.0f;

            if (score_x * score_y > score) {
                score = score_x * score_y;
                cornerIndex = index;
                flipped = false;
            }
        }
        if (allowFlip) {
            if (pos.x + size.y <= mBinSize.x && pos.y + size.x <= mBinSize.y) {
                float score_x = float(cornerBounds.x) / size.y;
                if (score_x > 1.0f)
                    score_x = 1.0f;
                float score_y = float(cornerBounds.y) / size.x;
                if (score_y > 1.0f)
                    score_y = 1.0f;

                if (score_x * score_y > score) {
                    score = score_x * score_y;
                    cornerIndex = index;
                    flipped = true;
                }
            }
        }
    }
    return { score, cornerIndex, flipped };
}

Atlas2::Entry Atlas2::insertIntoBin(Bin &bin, const Vector2i &_size, int cornerIndex, bool flipped)
{
    Vector2i size = _size;
    if (flipped)
        size = { size.y, size.x };

    Entry entry;
    entry.mArea.mTopLeft = bin.mCorners[cornerIndex] + bin.mOrigin;
    entry.mArea.mSize = size;
    entry.mFlipped = flipped;

    //Update x
    Vector2i newCornerX = bin.mCorners[cornerIndex] + Vector2i { size.x, 0 };
    while (cornerIndex > 0 && newCornerX.x >= bin.mCorners[cornerIndex - 1].x) {
        newCornerX.y = bin.mCorners[cornerIndex - 1].y;
        bin.mCorners.erase(bin.mCorners.begin() + (cornerIndex - 1));
        --cornerIndex;
    }
    bin.mCorners.insert(bin.mCorners.begin() + cornerIndex, newCornerX);
    ++cornerIndex;

    //Update y
    Vector2i newCornerY = bin.mCorners[cornerIndex] + Vector2i { 0, size.y };
    while (cornerIndex < bin.mCorners.size() - 1 && newCornerY.y >= bin.mCorners[cornerIndex + 1].y) {
        newCornerY.x = bin.mCorners[cornerIndex + 1].x;
        bin.mCorners.erase(bin.mCorners.begin() + (cornerIndex + 1));
    }
    bin.mCorners[cornerIndex] = newCornerY;

    return entry;
}

void Atlas2::addBin(const Vector2i &origin)
{
    mBins.emplace_back(Bin { origin }).mCorners.emplace_back(Vector2i { 0, 0 });
}

void Atlas2::clear()
{
    mBins.clear();
}

Atlas2::Entry Atlas2::insert(const Vector2i &size, const std::function<void()> &expand, bool allowFlip)
{
    assert(size.x <= mBinSize.x && size.y <= mBinSize.y);

    float score = 0;
    Bin *targetBin;
    int cornerIndex;
    bool flipped;

    for (Bin &bin : mBins) {
        auto [newScore, newCornerIndex, newFlipped] = getScore(bin, size, allowFlip);
        if (newScore > score) {
            score = newScore;
            cornerIndex = newCornerIndex;
            targetBin = &bin;
            flipped = newFlipped;
        }
    }

    if (score == 0) {
        size_t store = mBins.size();
        expand();
        if (mBins.size() > store) {
            return insert(size, expand, allowFlip);
        }
    }

    if (score > 0)
        return insertIntoBin(*targetBin, size, cornerIndex, flipped);
    else
        return Entry();
}

std::vector<Atlas2::Entry> Atlas2::insert(const std::span<Vector2i> &sizes, const std::function<void()> &expand, bool allowFlip)
{

    std::vector<std::pair<Vector2i, int>> items;

    int index = 0;

    std::ranges::transform(sizes, std::back_inserter(items), [&](const Vector2i &v) {
        return std::make_pair(v, index++);
    });

    std::ranges::sort(items, [](const std::pair<Vector2i, int> &first, const std::pair<Vector2i, int> &second) {
        if (first.first.x * first.first.y > second.first.x * second.first.y)
            return true;
        if (first.first.x * first.first.y < second.first.x * second.first.y)
            return false;
        return min(first.first.x, first.first.y) > min(second.first.x, second.first.y);
    });

    std::vector<Entry> result;
    result.resize(items.size());

    for (const std::pair<Vector2i, int> &p : items) {
        result[p.second] = insert(p.first, expand, allowFlip);
    }

    return result;
}

}