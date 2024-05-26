#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include "atom/system/module/memory.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)  ///< Begin DTO codegen section

namespace __class {
class MemorySlotClass;
}

/* Declare ObjectWrapper for your type */
/* Mapping-Enabled atom::system::MemoryInfo::MemorySlot */
typedef oatpp::data::mapping::type::Primitive<atom::system::MemoryInfo::MemorySlot,
                                              __class::MemorySlot>
    atom::system::MemoryInfo::MemorySlot;

namespace __class {

/**
 * Type info
 */
class MemorySlotClass {
private:
    /**
     * Type interpretation
     */
    class Inter
        : public oatpp::Type::Interpretation<
              atom::system::MemoryInfo::MemorySlot, oatpp::UnorderedFields<oatpp::String>> {
    public:
        oatpp::UnorderedFields<oatpp::String> interpret(
            const atom::system::MemoryInfo::MemorySlot& value) const override {
            return {{"capacity", value.capacity},
                    {"clockSpeed", value.clockSpeed},
                    {"type", value.type}};
        }

        atom::system::MemoryInfo::MemorySlot reproduce(
            const oatpp::UnorderedFields<oatpp::String> map) const override {
            return atom::system::MemoryInfo::MemorySlot(
                {map["capacity"], map["clockSpeed"], map["type"]});
        }
    };

public:
    static const oatpp::ClassId CLASS_ID;

    static oatpp::Type* getType() {
        static oatpp::Type type(CLASS_ID, nullptr, nullptr,
                         {{"system::memory",
                           new Inter()}} /* <-- Add type interpretation */);
        return &type;
    }
};

const oatpp::ClassId MemorySlotClass::CLASS_ID(
    "system::memory::atom::system::MemoryInfo::MemorySlot");

}  // namespace __class

#include OATPP_CODEGEN_END(DTO)  ///< End DTO codegen section
