#ifndef HISTOGRAM_HPP
#define HISTOGRAM_HPP


#include <string>
#include <vector>
#include <unordered_map>


class Histogram {
public:
    Histogram(void);
    Histogram(const std::string& fileName);

    void add(unsigned char r, unsigned char g, unsigned char b);
    void normalize(void);
    //const float*** getNormalized(void) const;

    void analyzeFile(const std::string& fileName);
    void writeToFile(const std::string& fileName);

    void createProfile(void);
    void loadProfileFromFile(const std::string& fileName);
    void saveProfileToFile(const std::string& fileName);

private:
    std::vector<std::vector<std::vector<unsigned>>> d_;
    std::vector<std::vector<std::vector<double>>> dn_;
    bool normalized_;

    struct ProfColor {
        int r, g, b;
        ProfColor(void) : r(-1), g(0), b(0) {}
        ProfColor(unsigned r, unsigned g, unsigned b) : r(r), g(g), b(b) {}
        ProfColor(unsigned rgb) : r(rgb&0x000000ff), g((rgb&0x0000ff00)>>8), b((rgb&0x00ff0000)>>16) {}
        operator int() const { return r | g<<8 | b<<16; }
    };

    struct ProfPos {
        int x, y;
        ProfPos(void) : x(0), y(0) {}
        ProfPos(int x, int y) : x(x), y(y) {}
        ProfPos(int xy) : x(xy&0x0000ffff), y((xy&0xffff0000)>>16) {}
        operator int() const { return (x&0x0000ffff) | (y&0x0000ffff)<<16; }
    };

    std::unordered_map<int, ProfPos> profile_;
    using InvProfile = std::unordered_map<int, std::pair<double, ProfColor>>;

    void createDefaultProfile(void);

    static void invProfilePressure(float& px, float& py, const InvProfile& invProf, int x, int y);
    static float invProfileError(const InvProfile& invProf,
                                 int x1, int y1, int x2, int y2);
    static void invProfileSwap(InvProfile& invProfile, int x1, int y1, int x2, int y2);
};


#endif // HISTOGRAM_HPP
