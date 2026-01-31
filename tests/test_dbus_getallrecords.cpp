#include "tests.hpp"
#include <QDBusArgument>
#include <QVariantMap>
#include <QVariantList>
#include <QString>
#include <cassert>
#include <cstdlib>
#include <cstddef>

namespace {

QVariantMap make_record_map(const char* id, const char* name, const char* path, const char* install_type) {
  QVariantMap m;
  m.insert(QStringLiteral("id"), QString::fromUtf8(id));
  m.insert(QStringLiteral("name"), QString::fromUtf8(name));
  m.insert(QStringLiteral("path"), QString::fromUtf8(path));
  m.insert(QStringLiteral("install_type"), QString::fromUtf8(install_type));
  return m;
}

int test_record_map_round_trip_via_qdbus_cast() {
  QVariantMap m = make_record_map("id1", "MyApp", "/opt/MyApp.AppImage", "Direct");
  QDBusArgument arg;
  arg << m;
  QVariant v = QVariant::fromValue(arg);
  QVariantMap m2 = qdbus_cast<QVariantMap>(v);
  assert(m2.value(QStringLiteral("id")).toString() == QStringLiteral("id1"));
  assert(m2.value(QStringLiteral("name")).toString() == QStringLiteral("MyApp"));
  assert(m2.value(QStringLiteral("path")).toString() == QStringLiteral("/opt/MyApp.AppImage"));
  assert(m2.value(QStringLiteral("install_type")).toString() == QStringLiteral("Direct"));
  return 0;
}

int test_to_map_empty_when_variant_holds_qdbus_argument() {
  QVariantMap m = make_record_map("x", "App", "/path/App.AppImage", "Downloaded");
  QDBusArgument arg;
  arg << m;
  QVariant v = QVariant::fromValue(arg);
  QVariantMap via_toMap = v.toMap();
  assert(via_toMap.isEmpty());
  QVariantMap via_qdbus_cast = qdbus_cast<QVariantMap>(v);
  assert(!via_qdbus_cast.isEmpty());
  assert(via_qdbus_cast.value(QStringLiteral("id")).toString() == QStringLiteral("x"));
  return 0;
}

int test_two_records_each_round_trip_via_qdbus_cast() {
  QVariantMap m1 = make_record_map("a", "First", "/first.AppImage", "GitHub");
  QVariantMap m2 = make_record_map("b", "Second", "/second.AppImage", "Downloaded");
  QDBusArgument arg1;
  arg1 << m1;
  QVariantMap m1_back = qdbus_cast<QVariantMap>(QVariant::fromValue(arg1));
  assert(m1_back.value(QStringLiteral("name")).toString() == QStringLiteral("First"));
  assert(m1_back.value(QStringLiteral("install_type")).toString() == QStringLiteral("GitHub"));
  QDBusArgument arg2;
  arg2 << m2;
  QVariantMap m2_back = qdbus_cast<QVariantMap>(QVariant::fromValue(arg2));
  assert(m2_back.value(QStringLiteral("name")).toString() == QStringLiteral("Second"));
  assert(m2_back.value(QStringLiteral("install_type")).toString() == QStringLiteral("Downloaded"));
  return 0;
}

using test_fn = int (*)();
static const test_fn tests[] = {
  test_record_map_round_trip_via_qdbus_cast,
  test_to_map_empty_when_variant_holds_qdbus_argument,
  test_two_records_each_round_trip_via_qdbus_cast,
};
static constexpr std::size_t num_tests = sizeof(tests) / sizeof(tests[0]);

}

std::size_t dbus_getallrecords_test_count() { return num_tests; }

int run_dbus_getallrecords_test(std::size_t i) {
  if (i >= num_tests) return EXIT_FAILURE;
  return tests[i]();
}

int run_dbus_getallrecords_tests() {
  for (std::size_t i = 0; i < num_tests; ++i)
    if (run_dbus_getallrecords_test(i) != 0) return EXIT_FAILURE;
  return EXIT_SUCCESS;
}
