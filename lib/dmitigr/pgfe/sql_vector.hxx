// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/sql_vector.hpp"
#include "dmitigr/pgfe/sql_string.hxx"

namespace dmitigr::pgfe::detail {

class iSql_vector : public Sql_vector {
public:
  iSql_vector() = default;

  iSql_vector(const std::string& input)
  {
    constexpr bool all{}; // Someday it might become an argument.
    const char* text{input.c_str()};
    while (*text != '\0') {
      const auto parsed = parse_sql_input(text);
      auto s = std::make_unique<iSql_string>(std::move(parsed.first));
      storage_.push_back(std::move(s));
      text = parsed.second;
    }
    DMINT_ASSERT(is_invariant_ok());
  }

  iSql_vector(const iSql_vector& rhs)
    : storage_(rhs.storage_.size())
  {
    std::transform(cbegin(rhs.storage_), cend(rhs.storage_), begin(storage_),
      [](const auto& sqlstr) { return sqlstr->clone(); });
    DMINT_ASSERT(is_invariant_ok());
  }

  iSql_vector& operator=(const iSql_vector& rhs)
  {
    iSql_vector tmp{rhs};
    swap(tmp);
    return *this;
  }

  iSql_vector(iSql_vector&& rhs) = default;

  iSql_vector& operator=(iSql_vector&& rhs) = default;

  void swap(iSql_vector& rhs) noexcept
  {
    storage_.swap(rhs.storage_);
  }

  // ---------------------------------------------------------------------------

  std::unique_ptr<Sql_vector> clone() const override
  {
    return std::make_unique<iSql_vector>(*this);
  }

  Container& container() override
  {
    return const_cast<Container&>(static_cast<const Sql_vector*>(this)->container());
  }

  const Container& container() const override
  {
    return storage_;
  }

  Value value(const std::size_t index) override
  {
    return const_cast<Value>(static_cast<const Sql_vector*>(this)->value(index));
  }

  const Value value(const std::size_t index) const override
  {
    DMINT_REQUIRE(index < container().size());
    return storage_[index].get();
  }

  // ---------------------------------------------------------------------------

  std::size_t sql_string_count() const override
  {
    return storage_.size();
  }

  bool has_sql_strings() const override
  {
    return !storage_.empty();
  }

  bool has_sql_string(const std::string& extra_name, const std::string& extra_value,
    const std::size_t offset = 0, const std::size_t extra_offset = 0) const override
  {
    return bool(sql_string_index(extra_name, extra_value, offset, extra_offset));
  }

  std::optional<std::size_t> sql_string_index(const std::string& extra_name, const std::string& extra_value,
    const std::size_t offset = 0, const std::size_t extra_offset = 0) const override
  {
    const auto i = container_const_iterator(extra_name, extra_value, offset, extra_offset);
    if (i != cend(storage_))
      return i - cbegin(storage_);
    else
      return std::nullopt;
  }

  std::size_t sql_string_index_throw(const std::string& extra_name, const std::string& extra_value,
    const std::size_t offset = 0, const std::size_t extra_offset = 0) const
  {
    const auto index = sql_string_index(extra_name, extra_value, offset, extra_offset);
    DMINT_REQUIRE(index);
    return *index;
  }

  Container::iterator container_iterator(const std::string& extra_name, const std::string& extra_value,
    const std::size_t offset = 0, const std::size_t extra_offset = 0) override
  {
    return container_iterator__(extra_name, extra_value, offset, extra_offset);
  }

  Container::const_iterator container_const_iterator(const std::string& extra_name, const std::string& extra_value,
    const std::size_t offset = 0, const std::size_t extra_offset = 0) const override
  {
    return container_iterator__(extra_name, extra_value, offset, extra_offset);
  }

  Sql_string* sql_string(const std::size_t index) override
  {
    return const_cast<Sql_string*>(static_cast<const Sql_vector*>(this)->sql_string(index));
  }

  const Sql_string* sql_string(const std::size_t index) const override
  {
    DMINT_REQUIRE(index < sql_string_count());
    return storage_[index].get();
  }

  Sql_string* sql_string(const std::string& extra_name, const std::string& extra_value,
    const std::size_t offset = 0, const std::size_t extra_offset = 0) override
  {
    return const_cast<Sql_string*>(static_cast<const Sql_vector*>(this)->
      sql_string(extra_name, extra_value, offset, extra_offset));
  }

  const Sql_string* sql_string(const std::string& extra_name, const std::string& extra_value,
    const std::size_t offset = 0, const std::size_t extra_offset = 0) const override
  {
    return sql_string(sql_string_index_throw(extra_name, extra_value, offset, extra_offset));
  }

  // ---------------------------------------------------------------------------

  std::string to_string() const override
  {
    std::string result;
    for (const auto& sql_string : storage_)
      result.append(sql_string->to_string());
    return result;
  }

  std::vector<std::unique_ptr<Sql_string>> to_vector() const override
  {
    iSql_vector copy{*this};
    return std::move(copy.storage_);
  }

protected:
  bool is_invariant_ok() const
  {
    return true;
  }

private:
  Container::iterator container_iterator__(const std::string& extra_name, const std::string& extra_value,
    const std::size_t offset = 0, const std::size_t extra_offset = 0) const
  {
    DMINT_REQUIRE(offset == 0 || offset < container().size());
    return std::find_if(begin(storage_) + offset, end(storage_),
      [&](const auto& sql_string)
      {
        if (const auto* const extra = sql_string ? sql_string->extra() : nullptr) {
          const auto index = extra->field_index(extra_name, extra_offset);
          return (index && (extra->data(*index)->bytes() == extra_value));
        } else
          return false;
      });
  }

  mutable std::vector<std::unique_ptr<Sql_string>> storage_;
};

} // namespace dmitigr::pgfe::detail
