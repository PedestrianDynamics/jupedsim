#pragma once

#include "CustomModelUpdate.hpp"

class ICustomModelDataImpl
{
public:
    virtual ~ICustomModelDataImpl() {};
    virtual std::string print() const = 0;
    virtual void apply_update(const CustomModelUpdate& upd) = 0;
};

class CustomModelData
{
public:
    explicit CustomModelData(std::shared_ptr<ICustomModelDataImpl> impl) : impl_(std::move(impl)) {}
    ~CustomModelData() = default;
    std::string print() const
    {
        if(impl_) {
            return impl_->print();
        }
        return "<CustomModelData: no implementation>";
    }

    void apply_update(const CustomModelUpdate& upd)
    {
        if(impl_) {
            impl_->apply_update(upd);
        }
    }

    std::shared_ptr<ICustomModelDataImpl> get_impl() const { return impl_; }

private:
    std::shared_ptr<ICustomModelDataImpl> impl_;
};

template <>
struct fmt::formatter<CustomModelData> : fmt::formatter<std::string> {
    template <typename FormatContext>
    auto format(const CustomModelData& data, FormatContext& ctx) const
    {
        std::string repr = data.print();
        return fmt::formatter<std::string>::format(repr, ctx);
    }
};