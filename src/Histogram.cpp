#include "Histogram.hpp"

#include <SFML/Graphics.hpp>
#include <algorithm>


Histogram::Histogram(void) :
    d_(256, std::vector<std::vector<unsigned>>(256, std::vector<unsigned>(256, 0))),
    dn_(256, std::vector<std::vector<double>>(256, std::vector<double>(256, 0.0))),
    normalized_(false)
{
    createDefaultProfile();
}

Histogram::Histogram(const std::string& fileName) :
    d_(256, std::vector<std::vector<unsigned>>(256, std::vector<unsigned>(256, 0))),
    dn_(256, std::vector<std::vector<double>>(256, std::vector<double>(256, 0.0))),
    normalized_(false)
{
    createDefaultProfile();
    analyzeFile(fileName);
}

void Histogram::add(unsigned char r, unsigned char g, unsigned char b) {
    ++d_[r][g][b];
    normalized_ = false;
}

void Histogram::normalize(void) {
    if (normalized_)
        return;

    unsigned max = 0;
    for (auto r=0u; r<256; ++r) {
        for (auto g=0u; g<256; ++g) {
            for (auto b=0u; b<256; ++b) {
                if (d_[r][g][b] > max)
                    max = d_[r][g][b];
            }
        }
    }

    printf("max: %u\n", max);
    double imax = 1.0/max;
    for (auto r=0u; r<256; ++r) {
        for (auto g=0u; g<256; ++g) {
            for (auto b=0u; b<256; ++b) {
                dn_[r][g][b] = pow(d_[r][g][b]*imax, 0.2);
            }
        }
    }

    normalized_ = true;
}

//const float*** Histogram::getNormalized(void) const {}

void Histogram::analyzeFile(const std::string& fileName) {
    sf::Image img;
    img.loadFromFile(fileName);

    auto s = img.getSize();
    auto* p = img.getPixelsPtr();

    for (auto y=0u; y<s.y; ++y) {
        for (auto x=0u; x<s.x; ++x) {
            unsigned long pp = (s.x*y + x)*4;
            ++d_[p[pp]][p[pp+1]][p[pp+2]];
        }
    }
}

void Histogram::writeToFile(const std::string& fileName) {
    normalize();

    sf::Image img;
    img.create(4096, 4096);

    for (auto r=0u; r<256; ++r) {
        for (auto g=0u; g<256; ++g) {
            for (auto b=0u; b<256; ++b) {
                auto& p = profile_[r | g<<8 | b<<16];
                unsigned c = dn_[r][g][b]*255;
                img.setPixel(p.x, p.y, sf::Color(c, c, c));
            }
        }
    }

    img.saveToFile(fileName);
}

void Histogram::createProfile(void) {
    normalize();

    InvProfile invProf;

    printf("Creating profile...\n");
    std::vector<std::pair<double, ProfColor>> dv;
    dv.reserve(16777216);

    for (auto r=0u; r<256; ++r)
        for (auto g=0u; g<256; ++g)
            for (auto b=0u; b<256; ++b)
                dv.emplace_back(dn_[r][g][b], ProfColor(r, g, b));

    std::sort(dv.begin(), dv.end(), [](auto& a, auto& b){ return a.first < b.first; });

    ProfPos p;
    for (auto it = dv.rbegin(); it != dv.rend(); ++it) {
        invProf[p] = *it;
        if (++p.x >= 4096) {
            ++p.y;
            p.x = 0;
        }
    }

    printf("Optimizing profile...\n");
    std::vector<bool> taken(4096, false);

    for (auto x=0; x<4094; ++x) {
        float minError = 3.40282347E+38F;
        auto minx = x+1;
        for (auto x2=x+1; x2<4096; ++x2) {
            float ne = invProfileError(invProf, x, 0, x2, 0);
            if (ne < minError) {
                minError = ne;
                minx = x2;
            }
        }
        invProfileSwap(invProf, x+1, 0, minx, 0);
    }

    for (auto y=0u; y<4095; ++y) {
        for (auto x=0u; x<4096; ++x) {
            float minError = 3.40282347E+38F;
            auto minx = x;
            for (auto x2=x; x2<4096; ++x2) {
                float ne = (invProfileError(invProf, x, y, x2, y+1) +
                           invProfileError(invProf, x-1, y, x2, y+1) +
                           invProfileError(invProf, x+1, y, x2, y+1))/3.0f;
                if (ne < minError) {
                    minError = ne;
                    minx = x2;
                }
            }
            invProfileSwap(invProf, x, y+1, minx, y+1);
        }
    }

    for (auto& pe : invProf)
        profile_[pe.second.second] = pe.first;
}

void Histogram::loadProfileFromFile(const std::string& fileName) {
    sf::Image img;
    img.loadFromFile(fileName);

    auto s = img.getSize();
    auto* p = img.getPixelsPtr();

    for (auto y=0u; y<s.y; ++y) {
        for (auto x=0u; x<s.x; ++x) {
            unsigned long pp = (s.x*y + x)*4;
            profile_[ProfColor(p[pp], p[pp+1], p[pp+2])] = ProfPos(x, y);
        }
    }
}

void Histogram::saveProfileToFile(const std::string& fileName) {
    sf::Image img;
    img.create(4096, 4096);

    for (auto r=0u; r<256; ++r) {
        for (auto g=0u; g<256; ++g) {
            for (auto b=0u; b<256; ++b) {
                auto& p = profile_[r | g<<8 | b<<16];
                img.setPixel(p.x, p.y, sf::Color(r, g, b));
            }
        }
    }

    img.saveToFile(fileName);
}

void Histogram::createDefaultProfile(void) {
    for (auto r=0u; r<256; ++r) {
        for (auto g=0u; g<256; ++g) {
            for (auto b=0u; b<256; ++b) {
                ProfColor c(r, g, b);
                ProfPos p;
                p.x = (r&0x0f) | ((g&0x0f)<<4) | ((b&0x0f)<<8);
                p.y = ((r&0xf0)>>4) | (g&0xf0) | ((b&0xf0)<<4);
                profile_[c] = p;
            }
        }
    }
}

void Histogram::invProfilePressure(float& px, float& py, const InvProfile& invProf, int x, int y) {
    px = 0.0f;
    py = 0.0f;

    px += invProfileError(invProf, x, y, x-1, y);
    px -= invProfileError(invProf, x, y, x+1, y);
}

float Histogram::invProfileError(const InvProfile& invProf,
                                           int x1, int y1, int x2, int y2) {
    if (x1 < 0) x1 = 0;
    if (x1 >= 4096) x1 = 4095;
    if (y1 < 0) y1 = 0;
    if (y1 >= 4096) y1 = 4095;
    if (x2 < 0) x2 = 0;
    if (x2 >= 4096) x2 = 4095;
    if (y2 < 0) y2 = 0;
    if (y2 >= 4096) y2 = 4095;

    ProfPos p1, p2;
    p1.x = x1;
    p1.y = y1;
    p2.x = x2;
    p2.y = y2;
    auto& c1 = invProf.at(p1).second;
    auto& c2 = invProf.at(p2).second;
    //auto& d1 = invProf.at(p1).first;
    //auto& d2 = invProf.at(p2).first;
    float rd(c2.r-c1.r), gd(c2.g-c1.g), bd(c2.b-c1.b);//, dd(d2-d1);

    return (rd*rd + gd*gd + bd*bd);//*dd*dd;
}

void Histogram::invProfileSwap(InvProfile& invProfile, int x1, int y1, int x2, int y2) {
    if (x1 < 0 || x1 >= 4096 || y1 < 0 || y1 >= 4096 ||
        x2 < 0 || x2 >= 4096 || y2 < 0 || y2 >= 4096)
        return;

    ProfPos p1(x1, y1), p2(x2, y2);
    auto temp = invProfile[p1];
    invProfile[p1] = invProfile[p2];
    invProfile[p2] = temp;
}
