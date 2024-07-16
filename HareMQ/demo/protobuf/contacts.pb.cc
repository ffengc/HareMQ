// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: contacts.proto

#include "contacts.pb.h"

#include <algorithm>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/wire_format_lite.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>

PROTOBUF_PRAGMA_INIT_SEG

namespace _pb = ::PROTOBUF_NAMESPACE_ID;
namespace _pbi = _pb::internal;

namespace contacts {
PROTOBUF_CONSTEXPR contact::contact(
    ::_pbi::ConstantInitialized)
  : name_(&::_pbi::fixed_address_empty_string, ::_pbi::ConstantInitialized{})
  , sn_(uint64_t{0u})
  , score_(0){}
struct contactDefaultTypeInternal {
  PROTOBUF_CONSTEXPR contactDefaultTypeInternal()
      : _instance(::_pbi::ConstantInitialized{}) {}
  ~contactDefaultTypeInternal() {}
  union {
    contact _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 contactDefaultTypeInternal _contact_default_instance_;
}  // namespace contacts
static ::_pb::Metadata file_level_metadata_contacts_2eproto[1];
static constexpr ::_pb::EnumDescriptor const** file_level_enum_descriptors_contacts_2eproto = nullptr;
static constexpr ::_pb::ServiceDescriptor const** file_level_service_descriptors_contacts_2eproto = nullptr;

const uint32_t TableStruct_contacts_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::contacts::contact, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  PROTOBUF_FIELD_OFFSET(::contacts::contact, sn_),
  PROTOBUF_FIELD_OFFSET(::contacts::contact, name_),
  PROTOBUF_FIELD_OFFSET(::contacts::contact, score_),
};
static const ::_pbi::MigrationSchema schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  { 0, -1, -1, sizeof(::contacts::contact)},
};

static const ::_pb::Message* const file_default_instances[] = {
  &::contacts::_contact_default_instance_._instance,
};

const char descriptor_table_protodef_contacts_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) =
  "\n\016contacts.proto\022\010contacts\"2\n\007contact\022\n\n"
  "\002sn\030\001 \001(\004\022\014\n\004name\030\002 \001(\t\022\r\n\005score\030\003 \001(\002b\006"
  "proto3"
  ;
static ::_pbi::once_flag descriptor_table_contacts_2eproto_once;
const ::_pbi::DescriptorTable descriptor_table_contacts_2eproto = {
    false, false, 86, descriptor_table_protodef_contacts_2eproto,
    "contacts.proto",
    &descriptor_table_contacts_2eproto_once, nullptr, 0, 1,
    schemas, file_default_instances, TableStruct_contacts_2eproto::offsets,
    file_level_metadata_contacts_2eproto, file_level_enum_descriptors_contacts_2eproto,
    file_level_service_descriptors_contacts_2eproto,
};
PROTOBUF_ATTRIBUTE_WEAK const ::_pbi::DescriptorTable* descriptor_table_contacts_2eproto_getter() {
  return &descriptor_table_contacts_2eproto;
}

// Force running AddDescriptors() at dynamic initialization time.
PROTOBUF_ATTRIBUTE_INIT_PRIORITY2 static ::_pbi::AddDescriptorsRunner dynamic_init_dummy_contacts_2eproto(&descriptor_table_contacts_2eproto);
namespace contacts {

// ===================================================================

class contact::_Internal {
 public:
};

contact::contact(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor();
  // @@protoc_insertion_point(arena_constructor:contacts.contact)
}
contact::contact(const contact& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  name_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    name_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (!from._internal_name().empty()) {
    name_.Set(from._internal_name(), 
      GetArenaForAllocation());
  }
  ::memcpy(&sn_, &from.sn_,
    static_cast<size_t>(reinterpret_cast<char*>(&score_) -
    reinterpret_cast<char*>(&sn_)) + sizeof(score_));
  // @@protoc_insertion_point(copy_constructor:contacts.contact)
}

inline void contact::SharedCtor() {
name_.InitDefault();
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  name_.Set("", GetArenaForAllocation());
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
::memset(reinterpret_cast<char*>(this) + static_cast<size_t>(
    reinterpret_cast<char*>(&sn_) - reinterpret_cast<char*>(this)),
    0, static_cast<size_t>(reinterpret_cast<char*>(&score_) -
    reinterpret_cast<char*>(&sn_)) + sizeof(score_));
}

contact::~contact() {
  // @@protoc_insertion_point(destructor:contacts.contact)
  if (auto *arena = _internal_metadata_.DeleteReturnArena<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>()) {
  (void)arena;
    return;
  }
  SharedDtor();
}

inline void contact::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  name_.Destroy();
}

void contact::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}

void contact::Clear() {
// @@protoc_insertion_point(message_clear_start:contacts.contact)
  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  name_.ClearToEmpty();
  ::memset(&sn_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&score_) -
      reinterpret_cast<char*>(&sn_)) + sizeof(score_));
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* contact::_InternalParse(const char* ptr, ::_pbi::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    uint32_t tag;
    ptr = ::_pbi::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // uint64 sn = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 8)) {
          sn_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // string name = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 18)) {
          auto str = _internal_mutable_name();
          ptr = ::_pbi::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(ptr);
          CHK_(::_pbi::VerifyUTF8(str, "contacts.contact.name"));
        } else
          goto handle_unusual;
        continue;
      // float score = 3;
      case 3:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 29)) {
          score_ = ::PROTOBUF_NAMESPACE_ID::internal::UnalignedLoad<float>(ptr);
          ptr += sizeof(float);
        } else
          goto handle_unusual;
        continue;
      default:
        goto handle_unusual;
    }  // switch
  handle_unusual:
    if ((tag == 0) || ((tag & 7) == 4)) {
      CHK_(ptr);
      ctx->SetLastTag(tag);
      goto message_done;
    }
    ptr = UnknownFieldParse(
        tag,
        _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
        ptr, ctx);
    CHK_(ptr != nullptr);
  }  // while
message_done:
  return ptr;
failure:
  ptr = nullptr;
  goto message_done;
#undef CHK_
}

uint8_t* contact::_InternalSerialize(
    uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:contacts.contact)
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  // uint64 sn = 1;
  if (this->_internal_sn() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteUInt64ToArray(1, this->_internal_sn(), target);
  }

  // string name = 2;
  if (!this->_internal_name().empty()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_name().data(), static_cast<int>(this->_internal_name().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "contacts.contact.name");
    target = stream->WriteStringMaybeAliased(
        2, this->_internal_name(), target);
  }

  // float score = 3;
  static_assert(sizeof(uint32_t) == sizeof(float), "Code assumes uint32_t and float are the same size.");
  float tmp_score = this->_internal_score();
  uint32_t raw_score;
  memcpy(&raw_score, &tmp_score, sizeof(tmp_score));
  if (raw_score != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteFloatToArray(3, this->_internal_score(), target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:contacts.contact)
  return target;
}

size_t contact::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:contacts.contact)
  size_t total_size = 0;

  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // string name = 2;
  if (!this->_internal_name().empty()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_name());
  }

  // uint64 sn = 1;
  if (this->_internal_sn() != 0) {
    total_size += ::_pbi::WireFormatLite::UInt64SizePlusOne(this->_internal_sn());
  }

  // float score = 3;
  static_assert(sizeof(uint32_t) == sizeof(float), "Code assumes uint32_t and float are the same size.");
  float tmp_score = this->_internal_score();
  uint32_t raw_score;
  memcpy(&raw_score, &tmp_score, sizeof(tmp_score));
  if (raw_score != 0) {
    total_size += 1 + 4;
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_cached_size_);
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData contact::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSizeCheck,
    contact::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*contact::GetClassData() const { return &_class_data_; }

void contact::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message* to,
                      const ::PROTOBUF_NAMESPACE_ID::Message& from) {
  static_cast<contact *>(to)->MergeFrom(
      static_cast<const contact &>(from));
}


void contact::MergeFrom(const contact& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:contacts.contact)
  GOOGLE_DCHECK_NE(&from, this);
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  if (!from._internal_name().empty()) {
    _internal_set_name(from._internal_name());
  }
  if (from._internal_sn() != 0) {
    _internal_set_sn(from._internal_sn());
  }
  static_assert(sizeof(uint32_t) == sizeof(float), "Code assumes uint32_t and float are the same size.");
  float tmp_score = from._internal_score();
  uint32_t raw_score;
  memcpy(&raw_score, &tmp_score, sizeof(tmp_score));
  if (raw_score != 0) {
    _internal_set_score(from._internal_score());
  }
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void contact::CopyFrom(const contact& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:contacts.contact)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool contact::IsInitialized() const {
  return true;
}

void contact::InternalSwap(contact* other) {
  using std::swap;
  auto* lhs_arena = GetArenaForAllocation();
  auto* rhs_arena = other->GetArenaForAllocation();
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &name_, lhs_arena,
      &other->name_, rhs_arena
  );
  ::PROTOBUF_NAMESPACE_ID::internal::memswap<
      PROTOBUF_FIELD_OFFSET(contact, score_)
      + sizeof(contact::score_)
      - PROTOBUF_FIELD_OFFSET(contact, sn_)>(
          reinterpret_cast<char*>(&sn_),
          reinterpret_cast<char*>(&other->sn_));
}

::PROTOBUF_NAMESPACE_ID::Metadata contact::GetMetadata() const {
  return ::_pbi::AssignDescriptors(
      &descriptor_table_contacts_2eproto_getter, &descriptor_table_contacts_2eproto_once,
      file_level_metadata_contacts_2eproto[0]);
}

// @@protoc_insertion_point(namespace_scope)
}  // namespace contacts
PROTOBUF_NAMESPACE_OPEN
template<> PROTOBUF_NOINLINE ::contacts::contact*
Arena::CreateMaybeMessage< ::contacts::contact >(Arena* arena) {
  return Arena::CreateMessageInternal< ::contacts::contact >(arena);
}
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>
