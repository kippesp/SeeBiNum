// SeeBiNum, see binary number

#include "precomp.h"

using Fixed12_12 = FixedNumber<int24_t, 12, 12>;
using Fixed16_16 = FixedNumber<int32_t, 16, 16>;
using Fixed8_24  = FixedNumber<int32_t, 8, 24>;

union NumberUnion
{
    uint8_t         buffer[8];
    uint8_t         ui8;
    uint16_t        ui16;
    uint32_t        ui32;
    uint64_t        ui64;
    int8_t          i8;
    int16_t         i16;
    int32_t         i32;
    int64_t         i64;
    float16_t       f16;
    float16m7e8s1_t f16m7e8s1;
    float32_t       f32;
    float64_t       f64;
    Fixed12_12      f12_12;
    Fixed16_16      f16_16;
    Fixed8_24       f8_24;

    NumberUnion() : ui64(0) {} // No initialization.
};

enum class ElementType : uint32_t
{
    Undefined = 0,
    Float32 = 1,    // starting from bit 0 - mantissa:23 exponent:8 sign:1
    Uint8 = 2,
    Int8 = 3,
    Uint16 = 4,
    Int16 = 5,
    Int32 = 6,
    Int64 = 7,
    StringChar8 = 8,
    Bool8 = 9,
    Float16m10e5s1 = 10, // mantissa:10 exponent:5 sign:1
    Float16 = Float16m10e5s1,
    Float64 = 11,   // mantissa:52 exponent:11 sign:1
    Uint32 = 12,
    Uint64 = 13,
    Complex64 = 14,
    Complex128 = 15,
    Float16m7e8s1 = 16,   // mantissa:7 exponent:8 sign:1
    Bfloat16 = Float16m7e8s1,
    Fixed24f12i12, // TODO: Make naming more consistent. Fixed24f12i12 vs Fixed12_12 vs f12_12
    Fixed32f16i16,
    Fixed32f24i8,
    Total,
};

struct NumberUnionAndType
{
    NumberUnion numberUnion;
    ElementType elementType;
};

struct Range
{
    uint32_t begin = 0;
    uint32_t end = 0;
};

template <typename T>
class Span
{
    T* begin_ = nullptr;
    T* end_ = nullptr;

public:
    Span() = default;
    Span(const Span&) = default;
    Span(Span&&) = default;

    Span(T* p, size_t s) : begin_(p), end_(begin_ + s) {}
    Span(T* b, T* e) : begin_(b), end_(e) {}

    const T* data() const noexcept { return begin_; }
    T* data() noexcept { return begin_; }
    size_t size() const noexcept { return end_ - begin_; }
    T* begin() const noexcept { return begin_; }
    T* end() const noexcept { return end_; }
    T* begin() noexcept { return begin_; }
    T* end() noexcept { return end_; }
    T& operator [](size_t index) noexcept { return begin_[index]; }
    const T& operator [](size_t index) const noexcept { return begin_[index]; }
    T& front() noexcept { return *begin_; }
    const T& front() const noexcept { return *begin_; }
    T& back() noexcept { return *(end_ - 1); }
    const T& back() const noexcept { return *(end_ - 1); }
    bool empty() const noexcept { return begin_ == end_; }

    void PopFront() { ++begin_; }
    void PopBack() { ++end_; }
};

enum class NumericOperationType : uint32_t
{
    None,
    Add,
    Subtract,
    Multiply,
    Divide,
    Dot,
    Total
};

struct NumericOperationAndRange
{
    NumericOperationType numericOperationType;
    Range range;
};

template <typename Enum>
bool ComparedMaskedFlags(Enum e, Enum mask, Enum value)
{
    static_assert(sizeof(Enum) == sizeof(uint32_t)); // Doesn't work with 64-bit enums.
    return (static_cast<uint32_t>(e) & static_cast<uint32_t>(mask)) == static_cast<uint32_t>(value);
}

template <typename Enum>
Enum SetFlags(Enum e, Enum clear, Enum set)
{
    return static_cast<Enum>((static_cast<uint32_t>(e) & ~static_cast<uint32_t>(clear)) | static_cast<uint32_t>(set));
}

////////////////////////////////////////////////////////////////////////////////

// Read data type and cast to double.
// The caller passes a data pointer of the given type.
/*static*/ double ReadToDouble(ElementType dataType, const void* data)
{
    double value = 0;
    switch (dataType)
    {
    case ElementType::Float32:          value =        *reinterpret_cast<const float*>(data);     break;
    case ElementType::Uint8:            value =        *reinterpret_cast<const uint8_t*>(data);   break;
    case ElementType::Int8:             value =        *reinterpret_cast<const int8_t*>(data);    break;
    case ElementType::Uint16:           value =        *reinterpret_cast<const uint16_t*>(data);  break;
    case ElementType::Int16:            value =        *reinterpret_cast<const int16_t*>(data);   break;
    case ElementType::Int32:            value =        *reinterpret_cast<const int32_t*>(data);   break;
    case ElementType::Int64:            value = double(*reinterpret_cast<const int64_t*>(data));  break;
    case ElementType::StringChar8:      value =        0; /* no numeric value for strings */      break;
    case ElementType::Bool8:            value =        *reinterpret_cast<const bool*>(data);      break;
    case ElementType::Float16:          value =        *reinterpret_cast<const float16_t*>(data); break;
    case ElementType::Bfloat16:         value =        *reinterpret_cast<const bfloat16_t*>(data);break;
    case ElementType::Float64:          value =        *reinterpret_cast<const double*>(data);    break;
    case ElementType::Uint32:           value =        *reinterpret_cast<const uint32_t*>(data);  break;
    case ElementType::Uint64:           value = double(*reinterpret_cast<const uint64_t*>(data)); break;
    case ElementType::Complex64:        throw std::invalid_argument("Complex64 type is not supported.");
    case ElementType::Complex128:       throw std::invalid_argument("Complex128 type is not supported.");
    case ElementType::Fixed24f12i12:    value =        *reinterpret_cast<const Fixed12_12*>(data); break;
    case ElementType::Fixed32f16i16:    value =        *reinterpret_cast<const Fixed16_16*>(data); break;
    case ElementType::Fixed32f24i8:     value =        *reinterpret_cast<const Fixed8_24*>(data);  break;
    }

    return value;
}

// Read data type and cast to int64.
// The caller passes a data pointer of the given type.
/*static*/ int64_t ReadToInt64(ElementType dataType, const void* data)
{
    int64_t value = 0;
    switch (dataType)
    {
    case ElementType::Float32:          value = int64_t(*reinterpret_cast<const float*>(data));     break;
    case ElementType::Uint8:            value = int64_t(*reinterpret_cast<const uint8_t*>(data));   break;
    case ElementType::Int8:             value = int64_t(*reinterpret_cast<const int8_t*>(data));    break;
    case ElementType::Uint16:           value = int64_t(*reinterpret_cast<const uint16_t*>(data));  break;
    case ElementType::Int16:            value = int64_t(*reinterpret_cast<const int16_t*>(data));   break;
    case ElementType::Int32:            value = int64_t(*reinterpret_cast<const int32_t*>(data));   break;
    case ElementType::Int64:            value = int64_t(*reinterpret_cast<const int64_t*>(data));   break;
    case ElementType::StringChar8:      value = int64_t(0); /* no numeric value for strings */      break;
    case ElementType::Bool8:            value = int64_t(*reinterpret_cast<const bool*>(data));      break;
    case ElementType::Float16:          value = int64_t(*reinterpret_cast<const float16_t*>(data)); break;
    case ElementType::Bfloat16:         value = int64_t(*reinterpret_cast<const bfloat16_t*>(data));break;
    case ElementType::Float64:          value = int64_t(*reinterpret_cast<const double*>(data));    break;
    case ElementType::Uint32:           value = int64_t(*reinterpret_cast<const uint32_t*>(data));  break;
    case ElementType::Uint64:           value = int64_t(*reinterpret_cast<const uint64_t*>(data));  break;
    case ElementType::Complex64:        throw std::invalid_argument("Complex64 type is not supported.");
    case ElementType::Complex128:       throw std::invalid_argument("Complex128 type is not supported.");
    case ElementType::Fixed24f12i12:    value = int64_t(*reinterpret_cast<const Fixed12_12*>(data));  break;
    case ElementType::Fixed32f16i16:    value = int64_t(*reinterpret_cast<const Fixed16_16*>(data));  break;
    case ElementType::Fixed32f24i8:;    value = int64_t(*reinterpret_cast<const Fixed8_24*>(data));   break;
    }

    return value;
}

// Read the raw bits, returned as int64.
// Useful for Unit Last Place comparisons.
/*static*/ int64_t ReadRawBitValue(ElementType dataType, const void* data)
{
    int64_t value = 0;
    switch (dataType)
    {
    case ElementType::Float32:          value = int64_t(*reinterpret_cast<const int32_t*>(data));   break;
    case ElementType::Uint8:            value = int64_t(*reinterpret_cast<const uint8_t*>(data));   break;
    case ElementType::Int8:             value = int64_t(*reinterpret_cast<const int8_t*>(data));    break;
    case ElementType::Uint16:           value = int64_t(*reinterpret_cast<const uint16_t*>(data));  break;
    case ElementType::Int16:            value = int64_t(*reinterpret_cast<const int16_t*>(data));   break;
    case ElementType::Int32:            value = int64_t(*reinterpret_cast<const int32_t*>(data));   break;
    case ElementType::Int64:            value = int64_t(*reinterpret_cast<const int64_t*>(data));   break;
    case ElementType::StringChar8:      value = int64_t(0); /* no numeric value for strings */      break;
    case ElementType::Bool8:            value = int64_t(*reinterpret_cast<const bool*>(data));      break;
    case ElementType::Float16:          value = int64_t(*reinterpret_cast<const int16_t*>(data));   break;
    case ElementType::Bfloat16:         value = int64_t(*reinterpret_cast<const int16_t*>(data));   break;
    case ElementType::Float64:          value = int64_t(*reinterpret_cast<const int64_t*>(data));   break;
    case ElementType::Uint32:           value = int64_t(*reinterpret_cast<const uint32_t*>(data));  break;
    case ElementType::Uint64:           value = int64_t(*reinterpret_cast<const uint64_t*>(data));  break;
    case ElementType::Complex64:        throw std::invalid_argument("Complex64 type is not supported.");
    case ElementType::Complex128:       throw std::invalid_argument("Complex128 type is not supported.");
    case ElementType::Fixed24f12i12:    value = int64_t(*reinterpret_cast<const int24_t*>(data));   break;
    case ElementType::Fixed32f16i16:    value = int64_t(*reinterpret_cast<const int32_t*>(data));   break;
    case ElementType::Fixed32f24i8:;    value = int64_t(*reinterpret_cast<const int32_t*>(data));   break;
    }

    return value;
}

// The caller passes a data pointer of the given type.
void WriteFromDouble(ElementType dataType, double value, /*out*/ void* data)
{
    switch (dataType)
    {
    case ElementType::Float32:          *reinterpret_cast<float*>(data)     = float(value);             break;
    case ElementType::Uint8:            *reinterpret_cast<uint8_t*>(data)   = uint8_t(value);           break;
    case ElementType::Int8:             *reinterpret_cast<int8_t*>(data)    = int8_t(value);            break;
    case ElementType::Uint16:           *reinterpret_cast<uint16_t*>(data)  = uint16_t(value);          break;
    case ElementType::Int16:            *reinterpret_cast<int16_t*>(data)   = int16_t(value);           break;
    case ElementType::Int32:            *reinterpret_cast<int32_t*>(data)   = int32_t(value);           break;
    case ElementType::Int64:            *reinterpret_cast<int64_t*>(data)   = int64_t(value);           break;
    case ElementType::StringChar8:      /* no change value for strings */                               break;
    case ElementType::Bool8:            *reinterpret_cast<bool*>(data)      = bool(value);              break;
    case ElementType::Float16:          *reinterpret_cast<uint16_t*>(data)  = half_float::detail::float2half<std::round_to_nearest, float>(float(value)); break;
    case ElementType::Bfloat16:         *reinterpret_cast<bfloat16_t*>(data)= bfloat16_t(float(value)); break;
    case ElementType::Float64:          *reinterpret_cast<double*>(data)    = value;                    break;
    case ElementType::Uint32:           *reinterpret_cast<uint32_t*>(data)  = uint32_t(value);          break;
    case ElementType::Uint64:           *reinterpret_cast<uint64_t*>(data)  = uint64_t(value);          break;
    case ElementType::Complex64:        throw std::invalid_argument("Complex64 type is not supported.");
    case ElementType::Complex128:       throw std::invalid_argument("Complex128 type is not supported.");
    case ElementType::Fixed24f12i12:    *reinterpret_cast<Fixed12_12*>(data) = float(value);            break;
    case ElementType::Fixed32f16i16:    *reinterpret_cast<Fixed16_16*>(data) = float(value);            break;
    case ElementType::Fixed32f24i8:;    *reinterpret_cast<Fixed8_24*>(data)  = float(value);            break;
    }

    // Use half_float::detail::float2half explicitly rather than the half constructor.
    // Otherwise values do not round-trip as expected.
    //
    // e.g. If you print float16 0x2C29, you get 0.0650024, but if you try to parse
    // 0.0650024, you get 0x2C28 instead. Then printing 0x2C28 shows 0.0649414,
    // but trying to parse 0.0649414 returns 0x2C27. Rounding to nearest fixes this.
}

// The caller passes a data pointer of the given type.
void WriteFromInt64(ElementType dataType, int64_t value, /*out*/ void* data)
{
    switch (dataType)
    {
    case ElementType::Float32:       *reinterpret_cast<float*>(data)     = float(value);              break;
    case ElementType::Uint8:         *reinterpret_cast<uint8_t*>(data)   = uint8_t(value);            break;
    case ElementType::Int8:          *reinterpret_cast<int8_t*>(data)    = int8_t(value);             break;
    case ElementType::Uint16:        *reinterpret_cast<uint16_t*>(data)  = uint16_t(value);           break;
    case ElementType::Int16:         *reinterpret_cast<int16_t*>(data)   = int16_t(value);            break;
    case ElementType::Int32:         *reinterpret_cast<int32_t*>(data)   = int32_t(value);            break;
    case ElementType::Int64:         *reinterpret_cast<int64_t*>(data)   = int64_t(value);            break;
    case ElementType::StringChar8:   /* no change value for strings */                                break;
    case ElementType::Bool8:         *reinterpret_cast<bool*>(data)      = bool(value);               break;
    case ElementType::Float16:       *reinterpret_cast<float16_t*>(data) = float16_t(float(value));   break;
    case ElementType::Bfloat16:      *reinterpret_cast<bfloat16_t*>(data)= bfloat16_t(float(value));  break;
    case ElementType::Float64:       *reinterpret_cast<double*>(data)    = double(value);             break;
    case ElementType::Uint32:        *reinterpret_cast<uint32_t*>(data)  = uint32_t(value);           break;
    case ElementType::Uint64:        *reinterpret_cast<uint64_t*>(data)  = uint64_t(value);           break;
    case ElementType::Complex64:     throw std::invalid_argument("Complex64 type is not supported.");
    case ElementType::Complex128:    throw std::invalid_argument("Complex128 type is not supported.");
    case ElementType::Fixed24f12i12: *reinterpret_cast<Fixed12_12*>(data) = float(value);             break;
    case ElementType::Fixed32f16i16: *reinterpret_cast<Fixed16_16*>(data) = float(value);             break;
    case ElementType::Fixed32f24i8:  *reinterpret_cast<Fixed8_24*>(data)  = float(value);             break;
    }
}

const char* g_elementTypeNames[] = {
    "undefined",    // Undefined = 0,
    "float32",      // Float32 = 1,
    "uint8",        // Uint8 = 2,
    "int8",         // Int8 = 3,
    "uint16",       // Uint16 = 4,
    "int16",        // Int16 = 5,
    "int32",        // Int32 = 6,
    "int64",        // Int64 = 7,
    "string8",      // StringChar8 = 8,
    "bool8",        // Bool = 9,
    "float16",      // Float16 = 10,
    "float64",      // Float64 = 11,
    "uint32",       // Uint32 = 12,
    "uint64",       // Uint64 = 13,
    "complex64",    // Complex64 = 14,
    "complex128",   // Complex128 = 15,
    "bfloat16",     // Float16m7e8s1 = 16,
    "fixed12_12",   // Fixed24f12i12 = 17,
    "fixed16_16",   // Fixed32f16i16 = 18,
    "fixed8_24",    // Fixed32f24i8 = 19,

};
static_assert(int(ElementType::Total) == 20 && std::size(g_elementTypeNames) == 20);

const static uint8_t g_byteSizeOfElementType[] = {
    0,  // Undefined = 0,
    4,  // Float32 = 1,
    1,  // Uint8 = 2,
    1,  // Int8 = 3,
    2,  // Uint16 = 4,
    2,  // Int16 = 5,
    4,  // Int32 = 6,
    8,  // Int64 = 7,
    0,  // sizeof(TypedBufferStringElement), // StringChar8 = 8,
    1,  // Bool = 9,
    2,  // Float16 = 10,
    8,  // Float64 = 11,
    4,  // Uint32 = 12,
    8,  // Uint64 = 13,
    8,  // Complex64 = 14,
    16, // Complex128 = 15,
    2,  // Float16m7e8s1 = 16,
    3,  // Fixed24f12i12 = 17,
    4,  // Fixed32f16i16 = 18,
    4,  // Fixed32f24i8 = 19,
};
static_assert(int(ElementType::Total) == 20 && std::size(g_byteSizeOfElementType) == 20);

const static uint8_t g_isFractionalElementType[] = {
    false, // Undefined = 0,
    true , // Float32 = 1,
    false, // Uint8 = 2,
    false, // Int8 = 3,
    false, // Uint16 = 4,
    false, // Int16 = 5,
    false, // Int32 = 6,
    false, // Int64 = 7,
    false, // sizeof(TypedBufferStringElement), // StringChar8 = 8,
    false, // Bool = 9,
    true , // Float16 = 10,
    true , // Float64 = 11,
    false, // Uint32 = 12,
    false, // Uint64 = 13,
    true , // Complex64 = 14,
    true , // Complex128 = 15,
    true , // Float16m7e8s1 = 16,
    true,  // Fixed24f12i12 = 17,
    true,  // Fixed32f16i16 = 18,
    true,  // Fixed32f24i8 = 19,
};
static_assert(int(ElementType::Total) == 20 && std::size(g_isFractionalElementType) == 20);

const static uint8_t g_isSignedElementType[] = {
    false, // Undefined = 0,
    true , // Float32 = 1,
    false, // Uint8 = 2,
    true , // Int8 = 3,
    false, // Uint16 = 4,
    true , // Int16 = 5,
    true , // Int32 = 6,
    true , // Int64 = 7,
    false, // sizeof(TypedBufferStringElement), // StringChar8 = 8,
    false, // Bool = 9,
    true , // Float16 = 10,
    true , // Float64 = 11,
    false, // Uint32 = 12,
    false, // Uint64 = 13,
    true , // Complex64 = 14,
    true , // Complex128 = 15,
    true , // Float16m7e8s1 = 16,
    true , // Fixed24f12i12 = 17,
    true , // Fixed32f16i16 = 18,
    true , // Fixed32f24i8 = 19,
};
static_assert(int(ElementType::Total) == 20 && std::size(g_isSignedElementType) == 20);

const char* g_numericOperationTypeNames[] = {
    "nop",
    "add",
    "subtract",
    "multiply",
    "divide",
    "dot",
};
static_assert(int(NumericOperationType::Total) == 6 && std::size(g_numericOperationTypeNames) == 6);

uint32_t GetSizeOfTypeInBytes(ElementType dataType) noexcept
{
    size_t index = static_cast<size_t>(dataType);
    return g_byteSizeOfElementType[index < std::size(g_byteSizeOfElementType) ? index : 0];
}

std::string_view GetTypeNameFromElementType(ElementType dataType) noexcept
{
    size_t index = static_cast<size_t>(dataType);
    return g_elementTypeNames[index < std::size(g_elementTypeNames) ? index : 0];
}

bool IsFractionalElementType(ElementType dataType) noexcept
{
    size_t index = static_cast<size_t>(dataType);
    return g_isFractionalElementType[index < std::size(g_isFractionalElementType) ? index : 0];
}

bool IsSignedElementType(ElementType dataType) noexcept
{
    size_t index = static_cast<size_t>(dataType);
    return g_isSignedElementType[index < std::size(g_isSignedElementType) ? index : 0];
}

std::string_view GetNumericOperationNameFromNumericOperationType(NumericOperationType numericOperationType) noexcept
{
    size_t index = static_cast<size_t>(numericOperationType);
    return g_numericOperationTypeNames[index < std::size(g_numericOperationTypeNames) ? index : 0];
}

std::string GetFormattedMessage(char const* formatString, ...)
{
    std::string formattedMessage;

    va_list argList;
    va_start(argList, formatString);
    char buffer[1000];
    buffer[0] = 0;

    vsnprintf(buffer, std::size(buffer), formatString, argList);
    formattedMessage.assign(buffer);
    return formattedMessage;
}

void AppendFormattedMessage(std::string& s, char const* formatString, ...)
{
    std::string formattedMessage;

    va_list argList;
    va_start(argList, formatString);
    char buffer[1000];
    buffer[0] = 0;

    vsnprintf(buffer, std::size(buffer), formatString, argList);
    s.append(buffer);
}

enum class NumericPrintingFlags : uint32_t
{
    Default = 0,
    ShowDataMask = 1,
    ShowDataAsHex = 0,
    ShowDataAsBinary = 1,
    ShowFloatMask = 2,
    ShowFloatAsDecimal = 0,
    ShowFloatAsHex = 2,
};

std::string GetPrintableNumericValue(
    ElementType elementType,
    const void* binaryData,
    std::string_view leftFlank,
    std::string_view rightFlank,
    NumericPrintingFlags valueFlags
    )
{
    const double value = ReadToDouble(elementType, binaryData);
    const int64_t rawBitValue = ReadRawBitValue(elementType, binaryData);

    const uint32_t sizeOfTypeInBytes = GetSizeOfTypeInBytes(elementType);
    const uint32_t countOfValueHexDigits = sizeOfTypeInBytes * 2;
    const uint64_t rawBitValueMask = ((1ui64 << (sizeOfTypeInBytes * 8 - 8)) << 8) - 1; // Shift twice to avoid wrap on x86/amd64 CPU's.

    const char* elementTypeName = GetTypeNameFromElementType(elementType).data();

    // Print formatted string. e.g.
    //
    //      float32 15361(0x46700400)

    std::string s = GetFormattedMessage("%10s ", elementTypeName);

    // Print numeric component.
    if (IsFractionalElementType(elementType))
    {
        if (ComparedMaskedFlags(valueFlags, NumericPrintingFlags::ShowFloatMask, NumericPrintingFlags::ShowFloatAsHex))
        {
            AppendFormattedMessage(/*inout*/ s, "%a", value);
        }
        else // NumericPrintingFlags::ShowDecimalFloat
        {
            AppendFormattedMessage(/*inout*/ s, "%.24g", value);
        }
    }
    else if (IsSignedElementType(elementType))
    {
        AppendFormattedMessage(/*inout*/ s, "%lld", rawBitValue);
    }
    else // unsigned
    {
        AppendFormattedMessage(/*inout*/ s, "%llu", rawBitValue);
    }

    s.append(leftFlank);

    // Print raw data, as binary or hex.
    if (ComparedMaskedFlags(valueFlags, NumericPrintingFlags::ShowDataMask, NumericPrintingFlags::ShowDataAsBinary))
    {
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(binaryData);
        for (size_t i = sizeOfTypeInBytes; i > 0;)
        {
            --i;
            uint32_t byte = bytes[i];
            AppendFormattedMessage(
                /*inout*/ s,
                "%c%c%c%c%c%c%c%c",
                (byte & 0x80 ? '1' : '0'),
                (byte & 0x40 ? '1' : '0'),
                (byte & 0x20 ? '1' : '0'),
                (byte & 0x10 ? '1' : '0'),
                (byte & 0x08 ? '1' : '0'),
                (byte & 0x04 ? '1' : '0'),
                (byte & 0x02 ? '1' : '0'),
                (byte & 0x01 ? '1' : '0') 
                );
        }
    }
    else if (ComparedMaskedFlags(valueFlags, NumericPrintingFlags::ShowDataMask, NumericPrintingFlags::ShowDataAsHex))
    {
        AppendFormattedMessage(/*inout*/ s, "0x%.*llX", countOfValueHexDigits, rawBitValue & rawBitValueMask);
    }
    // else nothing

    s.append(rightFlank);

    return s;
}

void PrintNumericType(
    ElementType elementType,
    const void* binaryData,
    std::string_view leftFlank,
    std::string_view rightFlank,
    NumericPrintingFlags numericPrintingFlags = NumericPrintingFlags::Default,
    ElementType elementTypeFilter = ElementType::Undefined
    )
{
    std::string s = GetPrintableNumericValue(elementType, binaryData, leftFlank, rightFlank, numericPrintingFlags);
    printf((elementType == elementTypeFilter) ? "   *%s\n" : "    %s\n", s.c_str());
}

void PrintBytes(const void* binaryData, size_t binaryDataByteSize)
{
    printf("         bytes ");

    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(binaryData);
    for (size_t i = 0; i < binaryDataByteSize; ++i)
    {
        printf("%02X ", bytes[i]);
    }

    printf("\n");
}

void PrintAllNumericTypesToBinary(
    double value,
    NumericPrintingFlags numericPrintingFlags = NumericPrintingFlags::Default,
    ElementType preferredElementType = ElementType::Undefined
    )
{
    constexpr std::string_view leftFlank = " -> ";
    constexpr std::string_view rightFlank = "";

    NumberUnion numberUnion = {};

    numberUnion.ui8  = static_cast<uint8_t >(value); PrintNumericType(ElementType::Uint8,  &numberUnion.ui8,  leftFlank, rightFlank, numericPrintingFlags, preferredElementType);
    numberUnion.ui16 = static_cast<uint16_t>(value); PrintNumericType(ElementType::Uint16, &numberUnion.ui16, leftFlank, rightFlank, numericPrintingFlags, preferredElementType);
    numberUnion.ui32 = static_cast<uint32_t>(value); PrintNumericType(ElementType::Uint32, &numberUnion.ui32, leftFlank, rightFlank, numericPrintingFlags, preferredElementType);
    numberUnion.ui64 = static_cast<uint64_t>(value); PrintNumericType(ElementType::Uint64, &numberUnion.ui64, leftFlank, rightFlank, numericPrintingFlags, preferredElementType);

    numberUnion.i8   = static_cast<int8_t >(value); PrintNumericType(ElementType::Int8,  &numberUnion.i8,  leftFlank, rightFlank, numericPrintingFlags, preferredElementType);
    numberUnion.i16  = static_cast<int16_t>(value); PrintNumericType(ElementType::Int16, &numberUnion.i16, leftFlank, rightFlank, numericPrintingFlags, preferredElementType);
    numberUnion.i32  = static_cast<int32_t>(value); PrintNumericType(ElementType::Int32, &numberUnion.i32, leftFlank, rightFlank, numericPrintingFlags, preferredElementType);
    numberUnion.i64  = static_cast<int64_t>(value); PrintNumericType(ElementType::Int64, &numberUnion.i64, leftFlank, rightFlank, numericPrintingFlags, preferredElementType);

    numberUnion.f16       = static_cast<float16_t>(float(value));  PrintNumericType(ElementType::Float16,  &numberUnion.f16, leftFlank, rightFlank, numericPrintingFlags, preferredElementType);
    numberUnion.f16m7e8s1 = static_cast<bfloat16_t>(float(value)); PrintNumericType(ElementType::Bfloat16, &numberUnion.f16m7e8s1, leftFlank, rightFlank, numericPrintingFlags, preferredElementType);
    numberUnion.f32       = static_cast<float32_t>(value);         PrintNumericType(ElementType::Float32,  &numberUnion.f32, leftFlank, rightFlank, numericPrintingFlags, preferredElementType);
    numberUnion.f64       = static_cast<float64_t>(value);         PrintNumericType(ElementType::Float64,  &numberUnion.f64, leftFlank, rightFlank, numericPrintingFlags, preferredElementType);

    numberUnion.f12_12 = static_cast<Fixed12_12>(float(value)); PrintNumericType(ElementType::Fixed24f12i12, &numberUnion.f12_12, leftFlank, rightFlank, numericPrintingFlags, preferredElementType);
    numberUnion.f16_16 = static_cast<Fixed16_16>(float(value)); PrintNumericType(ElementType::Fixed32f16i16, &numberUnion.f16_16, leftFlank, rightFlank, numericPrintingFlags, preferredElementType);
    numberUnion.f8_24  = static_cast<Fixed8_24>(float(value));  PrintNumericType(ElementType::Fixed32f24i8,  &numberUnion.f8_24,  leftFlank, rightFlank, numericPrintingFlags, preferredElementType);
}

void PrintAllNumericTypesFromBinary(
    int64_t value,
    NumericPrintingFlags numericPrintingFlags = NumericPrintingFlags::Default,
    ElementType preferredElementType = ElementType::Undefined
    )
{
    constexpr std::string_view leftFlank = " <- ";
    constexpr std::string_view rightFlank = "";

    NumberUnion numberUnion = {};
    numberUnion.i64 = value;

    PrintNumericType(ElementType::Uint8,  &numberUnion.ui8,  leftFlank, rightFlank, numericPrintingFlags, preferredElementType);
    PrintNumericType(ElementType::Uint16, &numberUnion.ui16, leftFlank, rightFlank, numericPrintingFlags, preferredElementType);
    PrintNumericType(ElementType::Uint32, &numberUnion.ui32, leftFlank, rightFlank, numericPrintingFlags, preferredElementType);
    PrintNumericType(ElementType::Uint64, &numberUnion.ui64, leftFlank, rightFlank, numericPrintingFlags, preferredElementType);
    
    PrintNumericType(ElementType::Int8,   &numberUnion.i8,  leftFlank, rightFlank, numericPrintingFlags, preferredElementType);
    PrintNumericType(ElementType::Int16,  &numberUnion.i16, leftFlank, rightFlank, numericPrintingFlags, preferredElementType);
    PrintNumericType(ElementType::Int32,  &numberUnion.i32, leftFlank, rightFlank, numericPrintingFlags, preferredElementType);
    PrintNumericType(ElementType::Int64,  &numberUnion.i64, leftFlank, rightFlank, numericPrintingFlags, preferredElementType);

    PrintNumericType(ElementType::Float16,  &numberUnion.f16, leftFlank, rightFlank, numericPrintingFlags, preferredElementType);
    PrintNumericType(ElementType::Bfloat16, &numberUnion.f16m7e8s1,      leftFlank, rightFlank, numericPrintingFlags, preferredElementType);
    PrintNumericType(ElementType::Float32,  &numberUnion.f32, leftFlank, rightFlank, numericPrintingFlags, preferredElementType);
    PrintNumericType(ElementType::Float64,  &numberUnion.f64, leftFlank, rightFlank, numericPrintingFlags, preferredElementType);

    PrintNumericType(ElementType::Fixed24f12i12, &numberUnion.f12_12, leftFlank, rightFlank, numericPrintingFlags, preferredElementType);
    PrintNumericType(ElementType::Fixed32f16i16, &numberUnion.f16_16, leftFlank, rightFlank, numericPrintingFlags, preferredElementType);
    PrintNumericType(ElementType::Fixed32f24i8,  &numberUnion.f8_24,  leftFlank, rightFlank, numericPrintingFlags, preferredElementType);
}

void PrintUsage()
{
    printf(
        "Usage:\n"
        "   seebinum 3.14159\n"
        "   seebinum -13\n"
        "   seebinum showbinary -13\n"
        "   seebinum showhex -13\n"
        "   seebinum 0x4240\n"
        "   seebinum 0b1101\n"
        "   seebinum float16 3.14\n"
        "   seebinum float16 raw 0x4240\n"
        "   seebinum uint32 mul 3 2 add 3 2 subtract 3 2 dot 1 2 3 4\n"
        "   seebinum float32 0x2.4p0\n"
        "   seebinum fixed12_12 sub 3 2\n"
        "\n"
        "Options:\n"
        "   showbinary showhex - display raw bits as binary or hex (default)\n"
        "   showhexfloat showdecfloat - display float as hex or decimal (default)\n"
        "   raw num - treat input as raw bit data or as number (default)\n"
        "   add subtract multiply divide dot - apply operation to following numbers\n"
        "   float16 bfloat16 float32 float64 - set floating point data type\n"
        "   uint8 uint16 uint32 uint64 int8 int16 int32 int64 - set integer data type\n"
        "   fixed12_12 fixed16_16 fixed8_24 - set fixed precision data type\n"
        "\n"
        "Dwayne Robinson, 2019-02-14..2019-03-29, No Copyright\r\n"
    );
}

void ParseNumber(
    _In_z_ const char* valueString,
    ElementType preferredElementType,
    bool parseAsRawData,
    _Out_ NumberUnionAndType& number
    )
{
    number = {};
    number.elementType = preferredElementType;

    // Read the numbers as float64 and int64.

    const bool isUndefinedType = (preferredElementType == ElementType::Undefined);
    const bool isFractionalType = IsFractionalElementType(preferredElementType);
    const bool parseFloatingPoint = isUndefinedType | isFractionalType;

    char* valueStringEnd = nullptr;
    double valueFloat = atof(valueString);
    int64_t valueInt = strtol(valueString, &valueStringEnd, 0);
    const bool isFractionalZero = (valueFloat == 0);

    // Parse binary (strtol doesn't appear to recognize standard C++ binary syntax?? e.g. 0b1101)
    if (valueInt == 0 && valueString[0] == '0' && valueString[1] == 'b')
    {
        valueInt = strtol(valueString + 2, &valueStringEnd, 2);
    }

    // If parsing as a float value failed, then try using the integer.
    if (isUndefinedType)
    {
        if (isFractionalZero && valueFloat == valueInt)
        {
            number.numberUnion.i64 = valueInt;
            number.elementType = ElementType::Uint64;
        }
        else
        {
            number.numberUnion.f64 = valueFloat;
            number.elementType = ElementType::Float64;
        }
    }
    else if (isFractionalType)
    {
        if (parseAsRawData)
        {
            number.numberUnion.i64 = valueInt;
        }
        else if (isFractionalZero)
        {
            WriteFromInt64(preferredElementType, valueInt, /*out*/ &number.numberUnion);
        }
        else
        {
            WriteFromDouble(preferredElementType, valueFloat, /*out*/ &number.numberUnion);
        }
    }
    else // integer
    {
        WriteFromInt64(preferredElementType, valueInt, /*out*/ &number.numberUnion);
    }
}

void PrintAllNumbers(
    Span<const NumberUnionAndType> numbers,
    NumericPrintingFlags numericPrintingFlags
    )
{
    constexpr std::string_view leftFlank = "(";
    constexpr std::string_view rightFlank = ")";

    for (auto& number : numbers)
    {
        if (number.elementType != ElementType::Undefined)
        {
            std::string s = GetPrintableNumericValue(number.elementType, &number.numberUnion, leftFlank, rightFlank, numericPrintingFlags);
            printf("    %s\r\n", s.c_str());
        }
    }
}

struct INumericOperationPerformer
{
    virtual void Add(Span<const NumberUnionAndType> numbers, _Out_ NumberUnionAndType& finalResult) = 0;
    virtual void Subtract(Span<const NumberUnionAndType> numbers, _Out_ NumberUnionAndType& finalResult) = 0;
    virtual void Multiply(Span<const NumberUnionAndType> numbers, _Out_ NumberUnionAndType& finalResult) = 0;
    virtual void Divide(Span<const NumberUnionAndType> numbers, _Out_ NumberUnionAndType& finalResult) = 0;
    virtual void Dot(Span<const NumberUnionAndType> numbers, _Out_ NumberUnionAndType& finalResult) = 0;
};

template <typename T, typename O>
T& CastAs(O& o)
{
    return reinterpret_cast<T&>(o);
}

template <typename T, typename O>
const T& CastAs(const O& o)
{
    return reinterpret_cast<const T&>(o);
}

template <typename T>
class NumericOperationPerformer : public INumericOperationPerformer
{
    void Add(Span<const NumberUnionAndType> numbers, _Out_ NumberUnionAndType& finalResult) override
    {
        T result = T(0);
        for (auto& n : numbers)
        {
            result += CastAs<T>(n);
        }
        CastAs<T>(finalResult) = result;
    }

    void Subtract(Span<const NumberUnionAndType> numbers, _Out_ NumberUnionAndType& finalResult) override
    {
        T result = T(0);
        if (!numbers.empty())
        {
            result = T(CastAs<T>(numbers.front()));
            numbers.PopFront();
            for (auto& n : numbers)
            {
                result -= CastAs<T>(n);
            }
        }
        CastAs<T>(finalResult) = result;
    };

    void Multiply(Span<const NumberUnionAndType> numbers, _Out_ NumberUnionAndType& finalResult) override
    {
        T result = T(1);
        for (auto& n : numbers)
        {
            result *= CastAs<T>(n);
        }
        CastAs<T>(finalResult) = result;
    };

    void Divide(Span<const NumberUnionAndType> numbers, _Out_ NumberUnionAndType& finalResult) override
    {
        T result = T(0);
        if (!numbers.empty())
        {
            result = T(CastAs<T>(numbers.front()));
            numbers.PopFront();
            for (auto& n : numbers)
            {
                result /= CastAs<T>(n);
            }
        }
        CastAs<T>(finalResult) = result;
    };

    void Dot(Span<const NumberUnionAndType> numbers, _Out_ NumberUnionAndType& finalResult) override
    {
        T result = T(0); 
        size_t i = 0;
        size_t numberCount = numbers.size();
        size_t evenNumberCount = numbers.size() & ~size_t(1);

        for (i = 0; i < evenNumberCount; i += 2)
        {
            T product = CastAs<T>(numbers[i]) * CastAs<T>(numbers[i + 1]);
            result += product;
        }
        if (i < numberCount)
        {
            result += CastAs<T>(numbers[i]);
        }
        CastAs<T>(finalResult) = result;

    #if 0 // Experimental rounding.
        constexpr uint64_t mantissaMaskLowBits = (0x1ui64 << 42) - 1;
        constexpr uint64_t mantissaMaskHighBits = ~mantissaMaskLowBits;
        constexpr uint64_t mantissaEvenBit = 0x1ui64 << 42;
        constexpr uint64_t mantissaMaskRoundingBit = 1ui64 << (42 - 1);

        // Round halves to evens
        NumberUnion result;
        result.f64 = double(0);
        size_t i = 0;
        size_t numberCount = numbers.size();
        size_t evenNumberCount = numbers.size() & ~size_t(1);

        for (i = 0; i < evenNumberCount; i += 2)
        {
            NumberUnion product;
            product.f64 = double(CastAs<T>(numbers[i])) * double(CastAs<T>(numbers[i + 1]));

            if ((product.ui64 & mantissaMaskLowBits) == mantissaMaskRoundingBit)
            {
                product.ui64 += (product.ui64 & mantissaEvenBit) ? -int64_t(mantissaMaskRoundingBit) : mantissaMaskRoundingBit;
            }
            result.f64 += product.f64;
            if ((result.ui64 & mantissaMaskLowBits) == mantissaMaskRoundingBit)
            {
                result.ui64 += (result.ui64 & mantissaEvenBit) ? -int64_t(mantissaMaskRoundingBit) : mantissaMaskRoundingBit;
            }
        }
        if (i < numberCount)
        {
            result.f64 += CastAs<T>(numbers[i]);
            if ((result.ui64 & mantissaMaskLowBits) == mantissaMaskRoundingBit)
            {
                result.ui64 += (result.ui64 & mantissaEvenBit) ? -int64_t(mantissaMaskRoundingBit) : mantissaMaskRoundingBit;
            }
        }
        CastAs<T>(finalResult) = T(float(result.f64));

        // Round halves towards infinity
        NumberUnion result;
        result.f64 = double(0);
        size_t i = 0;
        size_t numberCount = numbers.size();
        size_t evenNumberCount = numbers.size() & ~size_t(1);

        for (i = 0; i < evenNumberCount; i += 2)
        {
            NumberUnion product;
            product.f64 = double(CastAs<T>(numbers[i])) * double(CastAs<T>(numbers[i + 1]));
            product.ui64 += (0x1ui64 << 41);
            product.ui64 &= ~((0x1ui64 << 42) - 1);
            result.f64 += product.f64;
            result.ui64 += (0x1ui64 << 41);
            result.ui64 &= ~((0x1ui64 << 42) - 1);
        }
        if (i < numberCount)
        {
            result.f64 += CastAs<T>(numbers[i]);
            result.ui64 += (0x1ui64 << 41);
            result.ui64 &= ~((0x1ui64 << 42) - 1);
        }
        CastAs<T>(finalResult) = T(float(result.f64));
    #endif
    };
};

// Declare singletons since they are stateless anyway.
NumericOperationPerformer<float> g_numericOperationPerformerFloat32;
NumericOperationPerformer<double> g_numericOperationPerformerFloat64;
NumericOperationPerformer<float16_t> g_numericOperationPerformerFloat16;
NumericOperationPerformer<float16m7e8s1_t> g_numericOperationPerformerFloat16m7e8s1;
NumericOperationPerformer<uint8_t> g_numericOperationPerformerUint8;
NumericOperationPerformer<uint16_t> g_numericOperationPerformerUint16;
NumericOperationPerformer<uint32_t> g_numericOperationPerformerUint32;
NumericOperationPerformer<uint64_t> g_numericOperationPerformerUint64;
NumericOperationPerformer<int8_t> g_numericOperationPerformerInt8;
NumericOperationPerformer<int16_t> g_numericOperationPerformerInt16;
NumericOperationPerformer<int32_t> g_numericOperationPerformerInt32;
NumericOperationPerformer<int64_t> g_numericOperationPerformerInt64;
NumericOperationPerformer<Fixed12_12> g_numericOperationPerformerFixed24f12i12;
NumericOperationPerformer<Fixed16_16> g_numericOperationPerformerFixed32f16i16;
NumericOperationPerformer<Fixed8_24> g_numericOperationPerformerFixed32f24i8;

void PerformNumericOperation(
    NumericOperationType operation,
    Span<const NumberUnionAndType> numbers,
    _Out_ NumberUnionAndType& result
    )
{
    result = {};
    if (numbers.empty())
    {
        return;
    }

    INumericOperationPerformer* performer = &g_numericOperationPerformerFloat32;

    // Choose the respective operation performer based on data type.
    result.elementType = numbers.front().elementType;
    switch (result.elementType)
    {
    case ElementType::Undefined:        return;
    case ElementType::Float32:          performer = &g_numericOperationPerformerFloat32; break;
    case ElementType::Uint8:            performer = &g_numericOperationPerformerUint8; break;
    case ElementType::Int8:             performer = &g_numericOperationPerformerInt8; break;
    case ElementType::Uint16:           performer = &g_numericOperationPerformerUint16; break;
    case ElementType::Int16:            performer = &g_numericOperationPerformerInt16; break;
    case ElementType::Int32:            performer = &g_numericOperationPerformerInt32; break;
    case ElementType::Int64:            performer = &g_numericOperationPerformerInt64; break;
    case ElementType::StringChar8:      return;
    case ElementType::Bool8:            return;
    case ElementType::Float16:          performer = &g_numericOperationPerformerFloat16; break;
    case ElementType::Float64:          performer = &g_numericOperationPerformerFloat64; break;
    case ElementType::Uint32:           performer = &g_numericOperationPerformerUint32; break;
    case ElementType::Uint64:           performer = &g_numericOperationPerformerUint64; break;
    case ElementType::Complex64:        return;
    case ElementType::Complex128:       return;
    case ElementType::Float16m7e8s1:    performer = &g_numericOperationPerformerFloat16m7e8s1; break;
    case ElementType::Fixed24f12i12:    performer = &g_numericOperationPerformerFixed24f12i12; break;
    case ElementType::Fixed32f16i16:    performer = &g_numericOperationPerformerFixed32f16i16; break;
    case ElementType::Fixed32f24i8:     performer = &g_numericOperationPerformerFixed32f24i8; break;
    }

    switch (operation)
    {
    case NumericOperationType::Add:         performer->Add(numbers, /*out*/ result); break;
    case NumericOperationType::Subtract:    performer->Subtract(numbers, /*out*/ result); break;
    case NumericOperationType::Multiply:    performer->Multiply(numbers, /*out*/ result); break;
    case NumericOperationType::Divide:      performer->Divide(numbers, /*out*/ result); break;
    case NumericOperationType::Dot:         performer->Dot(numbers, /*out*/ result); break;
    }
}

int main(int argc, char* argv[])
{
    const char* valueString = "0";
    NumericPrintingFlags numericPrintingFlags = NumericPrintingFlags::Default;

    std::vector<NumericOperationAndRange> operations;
    std::vector<NumberUnionAndType> numbers;
    NumberUnionAndType numberUnionAndType;
    bool parseAsRawData = false;
    ElementType preferredElementType = ElementType::Undefined;

    if (argc >= 2)
    {
        for (size_t i = 1; i < size_t(argc); ++i)
        {
            using namespace std::literals::string_view_literals;
            std::string_view param(argv[i]);

            NumericOperationAndRange numericOperationAndRange = {};

            // Check if ordinary number or operator.
            if (isdigit(param.front()) || (param.front() == '-' && isdigit(param[1])))
            {
                auto value = param.begin();
                while (value != param.end())
                {
                    ParseNumber(&*value, preferredElementType, parseAsRawData, /*out*/ numberUnionAndType);
                    numbers.push_back(numberUnionAndType);
                    value = std::find(value, param.end(), ',');
                    if (value == param.end())
                    {
                        break;
                    }
                    else
                    {
                        ++value; // Skip the comma.
                    }
                }
            }
            else if (param == "nop"sv)
            {
                numericOperationAndRange.numericOperationType = NumericOperationType::None;
            }
            else if (param == "add"sv)
            {
                numericOperationAndRange.numericOperationType = NumericOperationType::Add;
            }
            else if (param == "sub"sv || param == "subtract"sv)
            {
                numericOperationAndRange.numericOperationType = NumericOperationType::Subtract;
            }
            else if (param == "mul"sv || param == "multiply"sv)
            {
                numericOperationAndRange.numericOperationType = NumericOperationType::Multiply;
            }
            else if (param == "div"sv || param == "divide"sv)
            {
                numericOperationAndRange.numericOperationType = NumericOperationType::Divide;
            }
            else if (param == "dot"sv || param == "dotproduct"sv)
            {
                numericOperationAndRange.numericOperationType = NumericOperationType::Dot;
            }
            else if (param == "raw"sv)
            {
                parseAsRawData = true;
            }
            else if (param == "num"sv)
            {
                parseAsRawData = false;
            }
            else if (param == "i32"sv || param == "int32"sv)
            {
                preferredElementType = ElementType::Int32;
            }
            else if (param == "ui32"sv || param == "uint32"sv)
            {
                preferredElementType = ElementType::Uint32;
            }
            else if (param == "i64"sv || param == "int64"sv)
            {
                preferredElementType = ElementType::Int64;
            }
            else if (param == "ui64"sv || param == "uint64"sv)
            {
                preferredElementType = ElementType::Uint64;
            }
            else if (param == "f16"sv || param == "float16"sv)
            {
                preferredElementType = ElementType::Float16;
            }
            else if (param == "f16m7e8s1"sv || param == "bfloat16"sv)
            {
                preferredElementType = ElementType::Float16m7e8s1;
            }
            else if (param == "f32"sv || param == "float32"sv)
            {
                preferredElementType = ElementType::Float32;
            }
            else if (param == "f64"sv || param == "float64"sv)
            {
                preferredElementType = ElementType::Float64;
            }
            else if (param == "fixed12_12"sv)
            {
                preferredElementType = ElementType::Fixed24f12i12;
            }
            else if (param == "fixed16_16"sv)
            {
                preferredElementType = ElementType::Fixed32f16i16;
            }
            else if (param == "fixed8_24"sv)
            {
                preferredElementType = ElementType::Fixed32f24i8;
            }
            else if (param == "showbinary"sv || param == "showbin"sv)
            {
                numericPrintingFlags = SetFlags(numericPrintingFlags, NumericPrintingFlags::ShowDataMask, NumericPrintingFlags::ShowDataAsBinary);
            }
            else if (param == "showhex"sv)
            {
                numericPrintingFlags = SetFlags(numericPrintingFlags, NumericPrintingFlags::ShowDataMask, NumericPrintingFlags::ShowDataAsHex);
            }
            else if (param == "showhexfloat"sv)
            {
                numericPrintingFlags = SetFlags(numericPrintingFlags, NumericPrintingFlags::ShowFloatMask, NumericPrintingFlags::ShowFloatAsHex);
            }
            else if (param == "showdecfloat"sv)
            {
                numericPrintingFlags = SetFlags(numericPrintingFlags, NumericPrintingFlags::ShowFloatMask, NumericPrintingFlags::ShowFloatAsDecimal);
            }
            else
            {
                printf("Unknown parameter: \"%.*s\"\r\n", int(param.size()), param.data());
                PrintUsage();
                return EXIT_FAILURE;
            }

            // Append any new numeric operations.
            if (numericOperationAndRange.numericOperationType != NumericOperationType::None)
            {
                const uint32_t numberCount = static_cast<uint32_t>(numbers.size());
                if (!operations.empty())
                {
                    operations.back().range.end = numberCount;
                }
                numericOperationAndRange.range.begin = numberCount;
                numericOperationAndRange.range.end = numberCount;
                operations.push_back(numericOperationAndRange);
            }
        }

        const uint32_t numberCount = static_cast<uint32_t>(numbers.size());
        if (!operations.empty())
        {
            operations.back().range.end = numberCount;
        }
    }
    else
    {
        PrintUsage();
        return EXIT_FAILURE;
    }

    if (!operations.empty())
    {
        // Process every operation in order.
        for (auto& operation : operations)
        {
            _Null_terminated_ const char* numericOperationName = GetNumericOperationNameFromNumericOperationType(operation.numericOperationType).data();

            // Print the operands.
            printf("Operands to %s:\r\n", numericOperationName);
            Span<const NumberUnionAndType> span(numbers.data() + operation.range.begin, numbers.data() + operation.range.end);
            PrintAllNumbers(span, numericPrintingFlags);

            // Process the values.
            NumberUnionAndType numberUnionAndType = {};
            PerformNumericOperation(operation.numericOperationType, span, /*out*/ numberUnionAndType);

            // Print the result.
            printf("Result from %s:\r\n", numericOperationName);
            Span<const NumberUnionAndType> singleElementSpan(&numberUnionAndType, 1);
            PrintAllNumbers(singleElementSpan, numericPrintingFlags);
            printf("\r\n");
        }
    }
    else if (numbers.size() == 1)
    {
        // If exactly one number is given, show it in possible formats.
        auto& numberUnion = numbers.front();
        double valueFloat = ReadToDouble(numberUnion.elementType, &numberUnion.numberUnion);
        int64_t valueInt = ReadToInt64(numberUnion.elementType, &numberUnion.numberUnion);

        printf("To binary:\r\n");
        PrintAllNumericTypesToBinary(valueFloat, numericPrintingFlags, preferredElementType);

        printf("\r\n" "From binary:\r\n");
        PrintAllNumericTypesFromBinary(valueInt, numericPrintingFlags, preferredElementType);
    }
    else if (!numbers.empty())
    {
        // If multiple numbers are given, print them all.
        PrintAllNumbers(Span<const NumberUnionAndType>(numbers.data(), numbers.size()), numericPrintingFlags);
    }

    return EXIT_SUCCESS;
}
