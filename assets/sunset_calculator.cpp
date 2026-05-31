/*
 * Calcul du levé, coucher de soleil et durée du jour
 * Basé sur l'algorithme de Jean Meeus (Astronomical Algorithms) et les formules NOAA Solar Calculator
 * Utilisation : ./sunset [latitude] [longitude] [YYYY-MM-DD]
 * Exemple     : ./sunset 48.8566 2.3522 2026-05-31
 */

#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

// Constantes
constexpr double PI = 3.14159265358979323846;
constexpr double TWO_PI = 2.0 * PI;
constexpr double DEG2RAD = PI / 180.0;
constexpr double RAD2DEG = 180.0 / PI;

// Zenith standard pour le lever/coucher (centre du disque + réfraction atmosphérique)
constexpr double ZENITH = 90.833; // degrés

// Utilitaires
double toRad(double deg)
{
  return deg * DEG2RAD;
}
double toDeg(double rad)
{
  return rad * RAD2DEG;
}

// Normalise un angle en degrés dans [0, 360)
double normalise360(double angle)
{
  angle = std::fmod(angle, 360.0);
  if (angle < 0)
    angle += 360.0;
  return angle;
}

// Formate un temps en secondes en HH:MM:SS
std::string formatSec(int sec)
{
  int h = sec / 3600;
  int ms = sec % 3600;
  int m = ms / 60;
  int s = ms % 60;
  std::ostringstream oss;
  oss << std::setw(2) << std::setfill('0') << h << ":" << std::setw(2) << std::setfill('0') << m << ":" << std::setw(2) << std::setfill('0') << s;
  return oss.str();
}

// Formate des minutes décimales en HH:MM:SS (heure locale UTC+offset)
std::string formatTime(double minutesUTC, int utcOffsetHours = 0)
{
  int sec = std::round(minutesUTC * 60 + utcOffsetHours * 3600);
  // Ramène dans [0, 86400]
  while (sec < 0)
    sec += 86400;
  while (sec >= 86400)
    sec -= 86400;
  return formatSec(sec);
}

// Durée du jour
std::string dayLength(double sunriseUTC, double sunsetUTC)
{
  int sec = std::round((sunsetUTC - sunriseUTC) * 60);
  return formatSec(sec);
}

// Jour Julien
double julianDay(int year, int month, int day)
{
  if (month <= 2)
  {
    year--;
    month += 12;
  }
  int A = year / 100;
  int B = 2 - A + A / 4;
  return std::floor(365.25 * (year + 4716)) + std::floor(30.6001 * (month + 1)) + day + B - 1524.5;
}

// Algorithme NOAA
struct SolarResult
{
  bool polarDay;     // soleil visible 24h
  bool polarNight;   // soleil invisible 24h
  double sunriseUTC; // minutes depuis minuit UTC
  double sunsetUTC;  // minutes depuis minuit UTC
  double solarNoonUTC;
};

SolarResult computeSolar(double latitude, double longitude, int year, int month, int day)
{
  SolarResult res{false, false, 0, 0, 0};

  double JD = julianDay(year, month, day);

  // Siècles juliens depuis J2000.0
  auto calcJC = [](double jd) { return (jd - 2451545.0) / 36525.0; };

  // Anomalie moyenne du Soleil
  auto calcGeomMeanLongSun = [](double t) { return normalise360(280.46646 + t * (36000.76983 + t * 0.0003032)); };
  auto calcGeomMeanAnomalySun = [](double t) { return normalise360(357.52911 + t * (35999.05029 - 0.0001537 * t)); };
  auto calcEccentricityEarthOrbit = [](double t) { return 0.016708634 - t * (0.000042037 + 0.0000001267 * t); };
  auto calcSunEqOfCenter = [&](double t) {
    double m = toRad(calcGeomMeanAnomalySun(t));
    double c = std::sin(m) * (1.914602 - t * (0.004817 + 0.000014 * t)) + std::sin(2 * m) * (0.019993 - 0.000101 * t) + std::sin(3 * m) * 0.000289;
    return c;
  };
  auto calcSunTrueLong = [&](double t) { return calcGeomMeanLongSun(t) + calcSunEqOfCenter(t); };
  auto calcSunApparentLong = [&](double t) {
    double o = calcSunTrueLong(t);
    double omega = 125.04 - 1934.136 * t;
    return o - 0.00569 - 0.00478 * std::sin(toRad(omega));
  };
  auto calcMeanObliquityOfEcliptic = [](double t) {
    double secs = 21.448 - t * (46.8150 + t * (0.00059 - t * 0.001813));
    return 23.0 + (26.0 + secs / 60.0) / 60.0;
  };
  auto calcObliquityCorrection = [&](double t) {
    double e0 = calcMeanObliquityOfEcliptic(t);
    double omega = 125.04 - 1934.136 * t;
    return e0 + 0.00256 * std::cos(toRad(omega));
  };
  auto calcSunDeclination = [&](double t) {
    double e = toRad(calcObliquityCorrection(t));
    double lambda = toRad(calcSunApparentLong(t));
    return toDeg(std::asin(std::sin(e) * std::sin(lambda)));
  };
  auto calcEquationOfTime = [&](double t) {
    double epsilon = toRad(calcObliquityCorrection(t));
    double l0 = toRad(calcGeomMeanLongSun(t));
    double e = calcEccentricityEarthOrbit(t);
    double m = toRad(calcGeomMeanAnomalySun(t));
    double y = std::tan(epsilon / 2.0);
    y *= y;
    double eqTime = y * std::sin(2 * l0) - 2 * e * std::sin(m) + 4 * e * y * std::sin(m) * std::cos(2 * l0) - 0.5 * y * y * std::sin(4 * l0) - 1.25 * e * e * std::sin(2 * m);
    return toDeg(eqTime) * 4.0; // en minutes
  };

  double JC = calcJC(JD);
  double eqTime = calcEquationOfTime(JC);
  double decl = calcSunDeclination(JC);

  // Angle horaire du coucher/lever
  double latRad = toRad(latitude);
  double declRad = toRad(decl);
  double zenRad = toRad(ZENITH);

  double cosHA = (std::cos(zenRad) - std::sin(latRad) * std::sin(declRad)) / (std::cos(latRad) * std::cos(declRad));

  if (cosHA > 1.0)
  {
    res.polarNight = true;
    return res;
  }
  if (cosHA < -1.0)
  {
    res.polarDay = true;
    return res;
  }

  double HA = toDeg(std::acos(cosHA)); // en degrés

  // Midi solaire vrai (UTC, minutes)
  res.solarNoonUTC = 720.0 - 4.0 * longitude - eqTime;

  // Lever et coucher (UTC, minutes depuis minuit)
  res.sunriseUTC = res.solarNoonUTC - HA * 4.0;
  res.sunsetUTC = res.solarNoonUTC + HA * 4.0;

  return res;
}

// Parsing de la date
void parseDate(const std::string &s, int &year, int &month, int &day)
{
  if (s.size() != 10 || s[4] != '-' || s[7] != '-')
    throw std::runtime_error("Format de date invalide (attendu YYYY-MM-DD)");
  year = std::stoi(s.substr(0, 4));
  month = std::stoi(s.substr(5, 2));
  day = std::stoi(s.substr(8, 2));
  if (month < 1 || month > 12 || day < 1 || day > 31)
    throw std::runtime_error("Date hors plage");
}

void local_time(time_t tt, int &y, int &mon, int &d, int &h, int &min, int &s, int &utc_offset, bool &is_dst)
{
  tm utc_tm, local_tm;
#ifdef _MSVC_LANG
  gmtime_s(&utc_tm, &tt);
  localtime_s(&local_tm, &tt);
#else
  utc_tm = *gmtime(&tt);
  local_tm = *localtime(&tt);
#endif

  y = local_tm.tm_year + 1900;
  mon = local_tm.tm_mon + 1;
  d = local_tm.tm_mday;
  h = local_tm.tm_hour;
  min = local_tm.tm_min;
  s = local_tm.tm_sec;
  utc_offset = (local_tm.tm_hour * 3600 + local_tm.tm_min * 60 + local_tm.tm_sec) - (utc_tm.tm_hour * 3600 + utc_tm.tm_min * 60 + utc_tm.tm_sec);
  is_dst = local_tm.tm_isdst == 1 ? true : false;
}

void local_time_now(int &y, int &mon, int &d, int &h, int &min, int &s, int &utc_offset, bool &is_dst)
{
  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  time_t tt = std::chrono::system_clock::to_time_t(now);
  std::cout << "TT: " << tt << std::endl;
  local_time(tt, y, mon, d, h, min, s, utc_offset, is_dst);
}

// Main
int main(int argc, char *argv[])
{
  // Position GPS de Paris : 48.8566 - 2.3522
  // Position GPS de Cugnaux : 43.542187 - 1.337237
  double latitude = 43.542187;
  double longitude = 1.337237;
  int year, month, day, hour, min, sec, utc_offset_sec;
  bool is_dst;
  int utcOffset;

  local_time_now(year, month, day, hour, min, sec, utc_offset_sec, is_dst);
  utcOffset = utc_offset_sec / 3600;

  if (argc >= 3)
  {
    try
    {
      latitude = std::stod(argv[1]);
      longitude = std::stod(argv[2]);
    }
    catch (...)
    {
      std::cerr << "Erreur : latitude/longitude invalides.\n";
      return 1;
    }
  }

  if (argc >= 4)
  {
    try
    {
      parseDate(argv[3], year, month, day);
    }
    catch (const std::exception &e)
    {
      std::cerr << "Erreur : " << e.what() << std::endl;
      return 1;
    }
  }

  if (argc >= 5)
  {
    try
    {
      utcOffset = std::stoi(argv[4]);
    }
    catch (...)
    {
      std::cerr << "Erreur : offset UTC invalide.\n";
      return 1;
    }
  }

  // Validation de la position
  if (latitude < -90 || latitude > 90)
  {
    std::cerr << "Erreur : latitude doit être dans [-90, 90].\n";
    return 1;
  }

  if (longitude < -180 || longitude > 180)
  {
    std::cerr << "Erreur : longitude doit être dans [-180, 180].\n";
    return 1;
  }

  SolarResult res = computeSolar(latitude, longitude, year, month, day);

  // Affichage
  std::cout << "Position  : " << std::fixed << std::setprecision(4) << latitude << "° N, " << longitude << "° E" << std::string(std::max(0, 14 - (int)std::to_string((int)std::abs(latitude)).size()), ' ') << std::endl;
  std::cout << "Date      : " << std::setw(4) << year << "-" << std::setw(2) << std::setfill('0') << month << "-" << std::setw(2) << std::setfill('0') << day << std::endl;
  std::cout << "Fuseau    : UTC" << (utcOffset >= 0 ? "+" : "") << utcOffset << std::endl;

  if (res.polarDay)
  {
    std::cout << "☀  Soleil de minuit (soleil visible 24h)" << std::endl;
  }
  else if (res.polarNight)
  {
    std::cout << "🌑  Nuit polaire (soleil invisible)" << std::endl;
  }
  else
  {
    std::cout << "Lever du soleil   : " << formatTime(res.sunriseUTC, utcOffset) << std::endl;
    std::cout << "Midi solaire      : " << formatTime(res.solarNoonUTC, utcOffset) << std::endl;
    std::cout << "Coucher du soleil : " << formatTime(res.sunsetUTC, utcOffset) << std::endl;
    std::cout << "Durée du jour     : " << dayLength(res.sunriseUTC, res.sunsetUTC) << std::endl;
    double flt_min = (double)(hour * 3600 + min * 60 + sec) / 60;
    if (is_dst)
      flt_min -= (double)utc_offset_sec / 60;
    std::cout << flt_min << std::endl;
    std::cout << "A " << formatTime(flt_min, utcOffset) << ", le soleil ";
    if (flt_min > res.sunriseUTC && flt_min < res.sunsetUTC)
      std::cout << "n'est pas";
    else
      std::cout << "est";
    std::cout << " couché." << std::endl;
  }

  return 0;
}
