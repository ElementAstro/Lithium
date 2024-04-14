#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include "atom/system/module/memory.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)  ///< Begin DTO codegen section

namespace __class {
class MemorySlotClass;
}

/* Declare ObjectWrapper for your type */
/* Mapping-Enabled Atom::System::MemoryInfo::MemorySlot */
typedef oatpp::data::mapping::type::Primitive<Atom::System::MemoryInfo::MemorySlot,
                                              __class::MemorySlot>
    Atom::System::MemoryInfo::MemorySlot;

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
              Atom::System::MemoryInfo::MemorySlot, oatpp::UnorderedFields<oatpp::String>> {
    public:
        oatpp::UnorderedFields<oatpp::String> interpret(
            const Atom::System::MemoryInfo::MemorySlot& value) const override {
            return {{"capacity", value.capacity},
                    {"clockSpeed", value.clockSpeed},
                    {"type", value.type}};
        }

        Atom::System::MemoryInfo::MemorySlot reproduce(
            const oatpp::UnorderedFields<oatpp::String> map) const override {
            return Atom::System::MemoryInfo::MemorySlot(
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
    "system::memory::Atom::System::MemoryInfo::MemorySlot");

}  // namespace __class

#include OATPP_CODEGEN_END(DTO)  ///< End DTO codegen section