#ifndef LEMON_UUID_HPP
#define LEMON_UUID_HPP

#include <random>
#include <memory>
#include <limits>
#include <cstddef>

#include <type_traits>



namespace lemon{ namespace uuids{
    struct uuid
    {
    public:
        typedef uint8_t value_type;
        typedef uint8_t& reference;
        typedef uint8_t const& const_reference;
        typedef uint8_t* iterator;
        typedef uint8_t const* const_iterator;
        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;

        // This does not work on some compilers
        // They seem to want the variable definec in
        // a cpp file
        //BOOST_STATIC_CONSTANT(size_type, static_size = 16);
        static constexpr size_type static_size() noexcept { return 16; }

    public:
        iterator begin() noexcept { return data; }
        const_iterator begin() const noexcept { return data; }
        iterator end() noexcept { return data+size(); }
        const_iterator end() const noexcept { return data+size(); }

        constexpr size_type size() const noexcept { return static_size(); }

        bool is_nil() const noexcept;

        enum variant_type
        {
            variant_ncs, // NCS backward compatibility
            variant_rfc_4122, // defined in RFC 4122 document
            variant_microsoft, // Microsoft Corporation backward compatibility
            variant_future // future definition
        };
        variant_type variant() const noexcept
        {
            // variant is stored in octet 7
            // which is index 8, since indexes count backwards
            unsigned char octet7 = data[8]; // octet 7 is array index 8
            if ( (octet7 & 0x80) == 0x00 ) { // 0b0xxxxxxx
                return variant_ncs;
            } else if ( (octet7 & 0xC0) == 0x80 ) { // 0b10xxxxxx
                return variant_rfc_4122;
            } else if ( (octet7 & 0xE0) == 0xC0 ) { // 0b110xxxxx
                return variant_microsoft;
            } else {
                //assert( (octet7 & 0xE0) == 0xE0 ) // 0b111xxxx
                return variant_future;
            }
        }

        enum version_type
        {
            version_unknown = -1,
            version_time_based = 1,
            version_dce_security = 2,
            version_name_based_md5 = 3,
            version_random_number_based = 4,
            version_name_based_sha1 = 5
        };
        version_type version() const noexcept
        {
            // version is stored in octet 9
            // which is index 6, since indexes count backwards
            uint8_t octet9 = data[6];
            if ( (octet9 & 0xF0) == 0x10 ) {
                return version_time_based;
            } else if ( (octet9 & 0xF0) == 0x20 ) {
                return version_dce_security;
            } else if ( (octet9 & 0xF0) == 0x30 ) {
                return version_name_based_md5;
            } else if ( (octet9 & 0xF0) == 0x40 ) {
                return version_random_number_based;
            } else if ( (octet9 & 0xF0) == 0x50 ) {
                return version_name_based_sha1;
            } else {
                return version_unknown;
            }
        }

        // note: linear complexity
        void swap(uuid& rhs) noexcept;

    public:
        // or should it be array<uint8_t, 16>
        uint8_t data[16];
    };

    inline bool operator== (uuid const& lhs, uuid const& rhs) noexcept
	{
		return memcmp(lhs.data, rhs.data, 16) == 0;
	}

    inline bool operator< (uuid const& lhs, uuid const& rhs) noexcept
	{
		return memcmp(lhs.data, rhs.data, 16) < 0;
	}

    inline bool operator!=(uuid const& lhs, uuid const& rhs) noexcept
    {
    return !(lhs == rhs);
    }

    inline bool operator>(uuid const& lhs, uuid const& rhs) noexcept
    {
    return rhs < lhs;
    }
    inline bool operator<=(uuid const& lhs, uuid const& rhs) noexcept
    {
    return !(rhs < lhs);
    }

    inline bool operator>=(uuid const& lhs, uuid const& rhs) noexcept
    {
    return !(lhs < rhs);
    }

    inline void swap(uuid& lhs, uuid& rhs) noexcept
    {
    lhs.swap(rhs);
    }

    // This is equivalent to boost::hash_range(u.begin(), u.end());
    inline std::size_t hash_value(uuid const& u) noexcept
    {
        std::size_t seed = 0;
        for(uuid::const_iterator i=u.begin(), e=u.end(); i != e; ++i)
        {
            seed ^= static_cast<std::size_t>(*i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }

        return seed;
    }

    namespace detail {
        inline char to_char(size_t i) {
            if (i <= 9) {
                return static_cast<char>('0' + i);
            } else {
                return static_cast<char>('a' + (i-10));
            }
        }

        inline wchar_t to_wchar(size_t i) {
            if (i <= 9) {
                return static_cast<wchar_t>(L'0' + i);
            } else {
                return static_cast<wchar_t>(L'a' + (i-10));
            }
        }
    }

    inline std::string to_string(uuid const& u)
    {
        std::string result;
        result.reserve(36);

        std::size_t i=0;
        for (uuid::const_iterator it_data = u.begin(); it_data!=u.end(); ++it_data, ++i) {
            const size_t hi = size_t(((*it_data) >> 4) & 0x0F);
            result += detail::to_char(hi);

            const size_t lo = size_t((*it_data) & 0x0F);
            result += detail::to_char(lo);

            if (i == 3 || i == 5 || i == 7 || i == 9) {
                result += '-';
            }
        }
        return result;
    }

    // generate a random-based uuid

    class random_generator {
    private:
        typedef std::uniform_int_distribution<unsigned long> distribution_type;

        struct null_deleter
        {
            void operator()(void const *) const {}
        };

    public:
        typedef uuid result_type;

        // default constructor creates the random number generator
        random_generator() = default;

        uuid operator()()
        {
            uuid u;

            int i=0;
            unsigned long random_value = _dist(_gen);
            for (uuid::iterator it=u.begin(); it!=u.end(); ++it, ++i) {
                if (i==sizeof(unsigned long)) {
                    random_value = _dist(_gen);
                    i = 0;
                }

                // static_cast gets rid of warnings of converting unsigned long to boost::uint8_t
                *it = static_cast<uuid::value_type>((random_value >> (i*8)) & 0xFF);
            }

            // set variant
            // must be 0b10xxxxxx
            *(u.begin()+8) &= 0xBF;
            *(u.begin()+8) |= 0x80;

            // set version
            // must be 0b0100xxxx
            *(u.begin()+6) &= 0x4F; //0b01001111
            *(u.begin()+6) |= 0x40; //0b01000000

            return u;
        }

    private:
        std::random_device              _gen;
        distribution_type               _dist;
    };
}}

namespace std {

    template <>
    struct is_pod<lemon::uuids::uuid> : true_type {};


	template<>
	struct hash<lemon::uuids::uuid>
	{
		size_t operator()(const lemon::uuids::uuid& _Keyval) const
		{	
			return lemon::uuids::hash_value(_Keyval);
		}
	};

}


#endif //LEMON_UUID_HPP