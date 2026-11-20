#include "lcarddiscovery.h"

#include "ltr11api.h"
#include "ltr12api.h"
#include "ltr22api.h"
#include "ltr24api.h"
#include "ltr25api.h"
#include "ltr27api.h"
#include "ltr35api.h"
#include "ltr41api.h"
#include "ltr42api.h"
#include "ltr43api.h"
#include "ltr51api.h"

#include "ltr114api.h"
#include "ltrapi.h"

#include <algorithm>
#include <array>
#include <format>
#include <string>
#include <vector>
#include <cstddef>

#ifdef _WIN32
#include <windows.h>
#endif

namespace qds
{

namespace
{

template<std::size_t N>
std::string serialString(const CHAR (&serial)[N])
{
  const auto end = std::find(
    std::begin(serial),
    std::end(serial),
    '\0');

  return {std::begin(serial), end};
}

template<std::size_t N>
std::string serialString(const std::array<CHAR, N>& serial)
{
  const auto end = std::find(
    serial.begin(),
    serial.end(),
    '\0');

  return {serial.begin(), end};
}

template<typename T, std::size_t N>
std::string serialString(const T (&serial)[N])
{
  const auto end = std::find(
    std::begin(serial),
    std::end(serial),
    0);

  return {
    reinterpret_cast<const char*>(serial),
    static_cast<std::size_t>(end - std::begin(serial))
  };
}

std::string readLtr11Serial(
  DWORD address,
  WORD port,
  const std::string& crateSerial,
  uint32_t slot)
{
  TLTR11 handle{};

  if (LTR11_Init(&handle) != LTR_OK)
    return {};

  const int err = LTR11_Open(
    &handle,
    address,
    port,
    crateSerial.c_str(),
    static_cast<INT>(slot));

  if (err != LTR_OK)
    return {};

  std::string result;

  if (LTR11_GetConfig(&handle) == LTR_OK)
    result = serialString(handle.ModuleInfo.Serial);

  LTR11_Close(&handle);

  return result;
}

std::string readLtr12Serial(
  DWORD address,
  WORD port,
  const std::string& crateSerial,
  uint32_t slot)
{
  TLTR12 handle{};

  if (LTR12_Init(&handle) != LTR_OK)
    return {};

  const int err = LTR12_Open(
    &handle,
    address,
    port,
    crateSerial.c_str(),
    static_cast<INT>(slot));

  if (err != LTR_OK)
  {
    if (LTR12_IsOpened(&handle) == LTR_OK)
      LTR12_Close(&handle);

    return {};
  }

  const std::string result =
    serialString(handle.ModuleInfo.Serial);

  LTR12_Close(&handle);

  return result;
}

std::string readLtr22Serial(
  DWORD address,
  WORD port,
  const std::string& crateSerial,
  uint32_t slot)
{
  TLTR22 handle{};

  if (LTR22_Init(&handle) != LTR_OK)
    return {};

  const int err = LTR22_Open(
    &handle,
    address,
    port,
    crateSerial.c_str(),
    static_cast<WORD>(slot));

  if (err != LTR_OK)
  {
    if (LTR22_IsOpened(&handle) == LTR_OK)
      LTR22_Close(&handle);

    return {};
  }

  std::string result;

  if (LTR22_GetModuleDescription(&handle) == LTR_OK)
    result = serialString(
      handle.ModuleInfo.Description.SerialNumber);

  LTR22_Close(&handle);

  return result;
}

std::string readLtr24Serial(
  DWORD address,
  WORD port,
  const std::string& crateSerial,
  uint32_t slot)
{
  TLTR24 handle{};

  if (LTR24_Init(&handle) != LTR_OK)
    return {};

  const int err = LTR24_Open(
    &handle,
    address,
    port,
    crateSerial.c_str(),
    static_cast<INT>(slot));

  if (err != LTR_OK)
  {
    if (LTR24_IsOpened(&handle) == LTR_OK)
      LTR24_Close(&handle);

    return {};
  }

  std::string result;

  if (LTR24_GetConfig(&handle) == LTR_OK)
    result = serialString(handle.ModuleInfo.Serial);

  LTR24_Close(&handle);

  return result;
}

std::string readLtr25Serial(
  DWORD address,
  WORD port,
  const std::string& crateSerial,
  uint32_t slot)
{
  TLTR25 handle{};

  if (LTR25_Init(&handle) != LTR_OK)
    return {};

  const int err = LTR25_Open(
    &handle,
    address,
    port,
    crateSerial.c_str(),
    static_cast<INT>(slot));

  if (err != LTR_OK)
  {
    if (LTR25_IsOpened(&handle) == LTR_OK)
      LTR25_Close(&handle);

    return {};
  }

  const std::string result =
    serialString(handle.ModuleInfo.Serial);

  LTR25_Close(&handle);

  return result;
}

std::string readLtr27Serial(
  DWORD address,
  WORD port,
  const std::string& crateSerial,
  uint32_t slot)
{
  TLTR27 handle{};

  if (LTR27_Init(&handle) != LTR_OK)
    return {};

  const int err = LTR27_Open(
    &handle,
    address,
    port,
    crateSerial.c_str(),
    static_cast<WORD>(slot));

  if (err != LTR_OK)
  {
    if (LTR27_IsOpened(&handle) == LTR_OK)
      LTR27_Close(&handle);

    return {};
  }

  std::string result;

  if (LTR27_GetDescription(
        &handle,
        LTR27_MODULE_DESCRIPTION) == LTR_OK)
  {
    result = serialString(
      handle.ModuleInfo.Module.SerialNumber);
  }

  LTR27_Close(&handle);

  return result;
}

std::string readLtr35Serial(
  DWORD address,
  WORD port,
  const std::string& crateSerial,
  uint32_t slot)
{
  TLTR35 handle{};

  if (LTR35_Init(&handle) != LTR_OK)
    return {};

  const int err = LTR35_Open(
    &handle,
    address,
    port,
    crateSerial.c_str(),
    static_cast<WORD>(slot));

  if (err != LTR_OK)
  {
    if (LTR35_IsOpened(&handle) == LTR_OK)
      LTR35_Close(&handle);

    return {};
  }

  const std::string result =
    serialString(handle.ModuleInfo.Serial);

  LTR35_Close(&handle);

  return result;
}

std::string readLtr41Serial(
  DWORD address,
  WORD port,
  const std::string& crateSerial,
  uint32_t slot)
{
  TLTR41 handle{};

  if (LTR41_Init(&handle) != LTR_OK)
    return {};

  const int err = LTR41_Open(
    &handle,
    static_cast<INT>(address),
    port,
    crateSerial.c_str(),
    static_cast<INT>(slot));

  if (err != LTR_OK)
  {
    if (LTR41_IsOpened(&handle) == LTR_OK)
      LTR41_Close(&handle);

    return {};
  }

  std::string result;

  if (LTR41_GetConfig(&handle) == LTR_OK)
    result = serialString(handle.ModuleInfo.Serial);

  LTR41_Close(&handle);

  return result;
}

std::string readLtr42Serial(
  DWORD address,
  WORD port,
  const std::string& crateSerial,
  uint32_t slot)
{
  TLTR42 handle{};

  if (LTR42_Init(&handle) != LTR_OK)
    return {};

  const int err = LTR42_Open(
    &handle,
    static_cast<INT>(address),
    port,
    crateSerial.c_str(),
    static_cast<INT>(slot));

  if (err != LTR_OK)
  {
    if (LTR42_IsOpened(&handle) == LTR_OK)
      LTR42_Close(&handle);

    return {};
  }

  std::string result;

  if (LTR42_GetConfig(&handle) == LTR_OK)
    result = serialString(handle.ModuleInfo.Serial);

  LTR42_Close(&handle);

  return result;
}

std::string readLtr43Serial(
  DWORD address,
  WORD port,
  const std::string& crateSerial,
  uint32_t slot)
{
  TLTR43 handle{};

  if (LTR43_Init(&handle) != LTR_OK)
    return {};

  const int err = LTR43_Open(
    &handle,
    address,
    port,
    crateSerial.c_str(),
    static_cast<INT>(slot));

  if (err != LTR_OK)
  {
    if (LTR43_IsOpened(&handle) == LTR_OK)
      LTR43_Close(&handle);

    return {};
  }

  std::string result;

  if (LTR43_GetConfig(&handle) == LTR_OK)
    result = serialString(handle.ModuleInfo.Serial);

  LTR43_Close(&handle);

  return result;
}

std::string readLtr51Serial(
  DWORD address,
  WORD port,
  const std::string& crateSerial,
  uint32_t slot)
{
  TLTR51 handle{};

  if (LTR51_Init(&handle) != LTR_OK)
    return {};

  const int err = LTR51_Open(
    &handle,
    address,
    port,
    crateSerial.c_str(),
    static_cast<INT>(slot),
    nullptr);

  if (err != LTR_OK)
  {
    if (LTR51_IsOpened(&handle) == LTR_OK)
      LTR51_Close(&handle);

    return {};
  }

  const std::string result =
    serialString(handle.ModuleInfo.Serial);

  LTR51_Close(&handle);

  return result;
}

std::string readLtr114Serial(
  DWORD address,
  WORD port,
  const std::string& crateSerial,
  uint32_t slot)
{
  TLTR114 handle{};

  if (LTR114_Init(&handle) != LTR_OK)
    return {};

  const int err = LTR114_Open(
    &handle,
    address,
    port,
    crateSerial.c_str(),
    static_cast<INT>(slot));

  if (err != LTR_OK)
    return {};

  std::string result;

  if (LTR114_GetConfig(&handle) == LTR_OK)
    result = serialString(handle.ModuleInfo.Serial);

  LTR114_Close(&handle);

  return result;
}

std::string errorString(int code)
{
  const char* text = LTR_GetErrorString(code);

  if (!text)
    return {};

#ifdef _WIN32
  const int length = MultiByteToWideChar(
    1251, 0, text, -1, nullptr, 0);

  if (length <= 0)
    return {};

  std::wstring wide(length, L'\0');

  if (!MultiByteToWideChar(
        1251, 0, text, -1, wide.data(), length))
    return {};

  const int size = WideCharToMultiByte(
    CP_UTF8, 0, wide.c_str(), -1,
    nullptr, 0, nullptr, nullptr);

  if (size <= 0)
    return {};

  std::string result(size, '\0');

  if (!WideCharToMultiByte(
        CP_UTF8, 0, wide.c_str(), -1,
        result.data(), size, nullptr, nullptr))
    return {};

  result.pop_back(); // Убираем завершающий '\0'
  return result;
#else
  return text;
#endif
}

std::string formatVersion(uint32_t version) {
  return std::format(
    "{}.{}.{}.{}",
    (version >> 24) & 0xff,
    (version >> 16) & 0xff,
    (version >> 8) & 0xff,
    version & 0xff);
}

class LtrConnection
{
public:
  LtrConnection()
  {
    m_initialized = LTR_Init(&m_handle) == LTR_OK;
  }

  ~LtrConnection()
  {
    if (m_initialized)
      LTR_Close(&m_handle);
  }

  LtrConnection(const LtrConnection&) = delete;
  LtrConnection& operator=(const LtrConnection&) = delete;

  bool initialized() const noexcept
  {
    return m_initialized;
  }

  TLTR* handle() noexcept
  {
    return &m_handle;
  }

private:
  TLTR m_handle{};
  bool m_initialized = false;
};

} // namespace

LCardDiscoveryResult LCardDiscovery::scan(
  uint32_t address,
  uint16_t port)
{
  LCardDiscoveryResult result;

  LtrConnection server;

  if (!server.initialized())
  {
    result.error = "Не удалось инициализировать LTR API";
    return result;
  }

  int err = LTR_OpenSvcControl(
    server.handle(),
    address,
    port);

  if (err != LTR_OK)
  {
    result.error = errorString(err);
    return result;
  }

  DWORD version = 0;

  err = LTR_GetServerVersion(
    server.handle(),
    &version);

  if (err == LTR_OK)
    result.serverVersion = formatVersion(version);

  DWORD found = 0;

  constexpr DWORD flags =
    LTR_GETCRATES_FLAGS_WORKMODE_ONLY;

  err = LTR_GetCratesEx(
    server.handle(),
    0,
    flags,
    &found,
    nullptr,
    nullptr,
    nullptr);

  if (err != LTR_OK)
  {
    result.error = errorString(err);
    return result;
  }

  if (found == 0)
    return result;

  using Serial = std::array<CHAR, LTR_CRATE_SERIAL_SIZE>;

  std::vector<Serial> serials(found);

  DWORD returned = 0;

  err = LTR_GetCratesEx(
    server.handle(),
    found,
    flags,
    &found,
    &returned,
    reinterpret_cast<CHAR (*)[LTR_CRATE_SERIAL_SIZE]>(
      serials.data()),
    nullptr);

  if (err != LTR_OK)
  {
    result.error = errorString(err);
    return result;
  }

  result.crates.reserve(returned);

  for (DWORD i = 0; i < returned; ++i)
  {
    LCardDiscoveredCrate crate;

    crate.serial = serialString(serials[i]);

    LtrConnection connection;

    if (!connection.initialized())
    {
      crate.error = "Не удалось инициализировать соединение";
      result.crates.push_back(std::move(crate));
      continue;
    }

    err = LTR_OpenCrate(
      connection.handle(),
      address,
      port,
      LTR_CRATE_IFACE_UNKNOWN,
      crate.serial.c_str());

    if (err != LTR_OK)
    {
      crate.error = errorString(err);
      result.crates.push_back(std::move(crate));
      continue;
    }

    TLTR_CRATE_INFO info{};

    err = LTR_GetCrateInfo(
      connection.handle(),
      &info);

    if (err == LTR_OK)
    {
      crate.typeId = info.CrateType;
      crate.interfaceId = info.CrateInterface;
    }
    else
    {
      crate.error = errorString(err);
    }

    TLTR_CRATE_DESCR descr{};

    err = LTR_GetCrateDescr(
      server.handle(),
      static_cast<INT>(crate.interfaceId),
      crate.serial.c_str(),
      &descr,
      sizeof(descr));

    if (err == LTR_OK)
    {
      crate.name = serialString(descr.devname);
      crate.typeName = serialString(descr.crate_type_name);
    }

    TLTR_CRATE_STATISTIC stat{};

    err = LTR_GetCrateStatistic(
      server.handle(),
      static_cast<INT>(crate.interfaceId),
      crate.serial.c_str(),
      &stat,
      sizeof(stat));

    if (err == LTR_OK)
    {
      if (stat.size >= offsetof(TLTR_CRATE_STATISTIC, mids))
        crate.slotCount = stat.modules_cnt;
    }

    std::array<WORD, LTR_MODULES_PER_CRATE_MAX> modules{};

    err = LTR_GetCrateModules(
      connection.handle(),
      modules.data());

    if (err != LTR_OK)
    {
      crate.error = errorString(err);
      result.crates.push_back(std::move(crate));
      continue;
    }

    for (std::size_t slot = 0;
         slot < modules.size();
         ++slot)
    {
      const WORD typeId = modules[slot];

      if (typeId == LTR_MID_EMPTY)
        continue;

      if (typeId == LTR_MID_IDENTIFYING ||
          typeId == LTR_MID_INVALID)
        continue;

      LCardDiscoveredModule module;

      module.slot = static_cast<uint32_t>(slot + 1);
      module.typeId = typeId;

      switch (typeId)
      {
      case LTR_MID_LTR11:
        module.serial = readLtr11Serial(
          address,
          port,
          crate.serial,
          module.slot);
        break;

      case LTR_MID_LTR12:
        module.serial = readLtr12Serial(
          address,
          port,
          crate.serial,
          module.slot);
        break;

      case LTR_MID_LTR22:
        module.serial = readLtr22Serial(
          address,
          port,
          crate.serial,
          module.slot);
        break;

      case LTR_MID_LTR24:
        module.serial = readLtr24Serial(
          address,
          port,
          crate.serial,
          module.slot);
        break;

      case LTR_MID_LTR25:
        module.serial = readLtr25Serial(
          address,
          port,
          crate.serial,
          module.slot);
        break;

      case LTR_MID_LTR27:
        module.serial = readLtr27Serial(
          address,
          port,
          crate.serial,
          module.slot);
        break;

      case LTR_MID_LTR35:
        module.serial = readLtr35Serial(
          address,
          port,
          crate.serial,
          module.slot);
        break;

      case LTR_MID_LTR41:
        module.serial = readLtr41Serial(
          address,
          port,
          crate.serial,
          module.slot);
        break;

      case LTR_MID_LTR42:
        module.serial = readLtr42Serial(
          address,
          port,
          crate.serial,
          module.slot);
        break;

      case LTR_MID_LTR43:
        module.serial = readLtr43Serial(
          address,
          port,
          crate.serial,
          module.slot);
        break;

      case LTR_MID_LTR51:
        module.serial = readLtr51Serial(
          address,
          port,
          crate.serial,
          module.slot);
        break;

      case LTR_MID_LTR114:
        module.serial = readLtr114Serial(
          address,
          port,
          crate.serial,
          module.slot);
        break;

      default:
        break;
      }

      crate.modules.push_back(std::move(module));
    }

    result.crates.push_back(std::move(crate));
  }

  return result;
}

std::string LCardDiscovery::crateName(uint32_t typeId)
{
  switch (typeId)
  {
  case LTR_CRATE_TYPE_UNKNOWN:    return "Unknown";
  case LTR_CRATE_TYPE_LTR010:     return "LTR-U-8/16";
  case LTR_CRATE_TYPE_LTR021:     return "LTR-U-1";
  case LTR_CRATE_TYPE_LTR030:     return "LTR-EU-8/16";
  case LTR_CRATE_TYPE_LTR031:     return "LTR-EU-2";
  case LTR_CRATE_TYPE_LTR032:     return "LTR-E-7/15";
  case LTR_CRATE_TYPE_LTR_CU_1:   return "LTR-CU-1";
  case LTR_CRATE_TYPE_LTR_CEU_1:  return "LTR-CEU-1";
  case LTR_CRATE_TYPE_BOOTLOADER: return "Bootloader";

  default:
    return std::format("Unknown ({})", typeId);
  }
}

std::string LCardDiscovery::crateName(
  uint32_t typeId,
  uint32_t slotCount)
{
  switch (typeId)
  {
  case LTR_CRATE_TYPE_UNKNOWN:
    return "Unknown";

  case LTR_CRATE_TYPE_LTR010:
    switch (slotCount)
    {
    case 8:  return "LTR-U-8";
    case 16: return "LTR-U-16";
    default: return "LTR-U-8/16";
    }

  case LTR_CRATE_TYPE_LTR021:
    return "LTR-U-1";

  case LTR_CRATE_TYPE_LTR030:
    switch (slotCount)
    {
    case 8:  return "LTR-EU-8";
    case 16: return "LTR-EU-16";
    default: return "LTR-EU-8/16";
    }

  case LTR_CRATE_TYPE_LTR031:
    return "LTR-EU-2";

  case LTR_CRATE_TYPE_LTR032:
    switch (slotCount)
    {
    case 7:  return "LTR-E-7";
    case 15: return "LTR-E-15";
    default: return "LTR-E-7/15";
    }

  case LTR_CRATE_TYPE_LTR_CU_1:
    return "LTR-CU-1";

  case LTR_CRATE_TYPE_LTR_CEU_1:
    return "LTR-CEU-1";

  case LTR_CRATE_TYPE_BOOTLOADER:
    return "Bootloader";

  default:
    return std::format("Unknown ({})", typeId);
  }
}

std::string LCardDiscovery::moduleName(uint32_t typeId)
{
  switch (typeId)
  {
  case LTR_MID_LTR11:  return "LTR11";
  case LTR_MID_LTR12:  return "LTR12";
  case LTR_MID_LTR22:  return "LTR22";
  case LTR_MID_LTR24:  return "LTR24";
  case LTR_MID_LTR25:  return "LTR25";
  case LTR_MID_LTR27:  return "LTR27";
  case LTR_MID_LTR35:  return "LTR35";
  case LTR_MID_LTR41:  return "LTR41";
  case LTR_MID_LTR42:  return "LTR42";
  case LTR_MID_LTR43:  return "LTR43";
  case LTR_MID_LTR51:  return "LTR51";
  case LTR_MID_LTR114: return "LTR114";

  default:
    return std::format("Unknown ({})", typeId);
  }
}

} // namespace qds