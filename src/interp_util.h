inline uint64_t mulhu(uint64_t a, uint64_t b) {
    uint64_t t;
    uint32_t y1, y2, y3;
    uint64_t a0 = (uint32_t)a, a1 = a >> 32;
    uint64_t b0 = (uint32_t)b, b1 = b >> 32;
    t = a1*b0 + ((a0*b0) >> 32);
    y1 = t;
    y2 = t >> 32;
    t = a0*b1 + y1;
    y1 = t;
    t = a1*b1 + y2 + (t >> 32);
    y2 = t;
    y3 = t >> 32;
    return ((uint64_t)y3 << 32) | y2;
}

inline int64_t mulh(int64_t a, int64_t b) {
    int negate = (a < 0) != (b < 0);
    uint64_t res = mulhu(a < 0 ? -a : a, b < 0 ? -b : b);
    return negate ? ~res + (a * b == 0) : res;
}

inline int64_t mulhsu(int64_t a, uint64_t b) {
    int negate = a < 0;
    uint64_t res = mulhu(a < 0 ? -a : a, b);
    return negate ? ~res + (a * b == 0) : res;
}

inline uint64_t div1(uint64_t a, uint64_t b) {
    uint64_t rd = 0;
    if (b == 0) {
        rd = UINT64_MAX;
    }
    else if (a == INT64_MIN && b == UINT64_MAX) {
        rd = INT64_MIN;
    }
    else {
        rd = (int64_t)a / (int64_t)b;
    }
    return rd;
}

inline uint64_t divu(uint64_t a, uint64_t b) {
    uint64_t rd = 0;
    if (b == 0) {
        rd = UINT64_MAX;
    }
    else {
        rd = a / b;
    }
    return rd;
}

inline uint64_t rem(uint64_t a, uint64_t b) {
    uint64_t rd = 0;
    if (b == 0) {
        rd = a;
    }
    else if (a == INT64_MIN && b == UINT64_MAX) {
        rd = 0;
    }
    else {
        rd = (int64_t)a % (int64_t)b;
    }
    return rd;
}

inline uint64_t remu(uint64_t a, uint64_t b) {
    uint64_t rd = 0;
    if (b == 0) {
        rd = a;
    }
    else {
        rd = a % b;
    }
    return rd;
}
