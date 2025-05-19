#ifndef PASSANALYZER_H
#define PASSANALYZER_H

#include <QElapsedTimer>
#include <Qstring>
#include <Windows.h>
#include <qdebug.h>
#include <string>

namespace {
// ASCII extended, CP866
constexpr uint8_t SPECIAL = 15 + 7 + 6 + 4;      // [33; 47] | [58; 64] | [91; 96] |  [123; 126]
constexpr uint8_t DIGITS = 10;                   // [48; 57]
constexpr uint8_t UPR_LAT_LETTERS = 26;          // [65; 90]
constexpr uint8_t LWR_LAT_LETTERS = 26;          // [97; 122]
constexpr uint8_t UPR_CYR_LETTERS = 32 + 1;      // [128; 159] | 240
constexpr uint8_t LWR_CYR_LETTERS = 16 + 16 + 1; // [160; 175] | [224; 239] | 241

// Variables for hacking time
constexpr uint32_t SECS_FOR_COMBINATION = 2;
constexpr uint32_t ATTEMPTS_BEFORE_LOCK = 3;
constexpr uint32_t SECS_FOR_UNLOCK = 1;

// Max value 2^64
constexpr uint64_t UINT64_MAX_VALUE = (std::numeric_limits<uint64_t>::max)();

} // namespace

uint32_t countAlphabetPower(const uint8_t bitMask)
{
    uint32_t alphPower = 0;

    if (bitMask & (1 << 0))
        alphPower += SPECIAL;

    if (bitMask & (1 << 1))
        alphPower += DIGITS;

    if (bitMask & (1 << 2))
        alphPower += UPR_LAT_LETTERS;

    if (bitMask & (1 << 3))
        alphPower += LWR_LAT_LETTERS;

    if (bitMask & (1 << 4))
        alphPower += UPR_CYR_LETTERS;

    if (bitMask & (1 << 5))
        alphPower += LWR_CYR_LETTERS;

    return alphPower;
}

// Fast (binary) exponentiation method
uint64_t doBinExp(uint64_t base, uint32_t power)
{
    uint64_t res = 1; // if power == 0

    while (power > 0) // till last bit
    {
        if (power % 2 == 1) // check last bit
        {
            if (res >= UINT64_MAX_VALUE / base)

                throw std::overflow_error("Exponentiation \"result\" variable overflow");

            res *= base;
        }

        if (base >= UINT64_MAX_VALUE / base)

            throw std::overflow_error("Exponentiation \"base\" variable overflow");

        power /= 2;   // dispose of last bit
        base *= base; // iterate through each of binary places
    }

    qInfo() << "Combinations quantity: " << res << "";
    return res;
}

void countHackTime(uint64_t cmbnQnty,
                   uint32_t secsForAttempt,
                   uint32_t attemptsBeforeLock,
                   uint32_t lockTime)
{
    const uint32_t SECS_YEAR = 365 * 24 * 60 * 60;
    const uint32_t SECS_DAY = 24 * 60 * 60;
    const uint32_t SECS_HOURS = 60 * 60;
    const uint32_t SECS_MINUTES = 60;

    uint64_t totalSecs = 0;

    if (cmbnQnty > UINT64_MAX_VALUE / secsForAttempt) {
        qInfo() << "Overflow in multiplying secs and combinations. Setting total secs to "
                   "UINT64_MAX_VALUE.";
        totalSecs = UINT64_MAX_VALUE;
    } else {
        totalSecs = cmbnQnty * secsForAttempt;
    }

    uint64_t lockPenalty = 0;
    if (cmbnQnty / attemptsBeforeLock > UINT64_MAX_VALUE / lockTime) {
        qInfo() << "Overflow in lock penalty. Setting lock penalty to UINT64_MAX_VALUE.";
        lockPenalty = UINT64_MAX_VALUE;
    } else {
        lockPenalty = (cmbnQnty / attemptsBeforeLock) * lockTime;
    }

    if (totalSecs > UINT64_MAX_VALUE - lockPenalty) {
        qInfo() << "Overflow in suming lock penalty and secs for all attempts. Setting total "
                   "secs to UINT64_MAX_VALUE.";
        totalSecs = UINT64_MAX_VALUE;
    } else {
        totalSecs += lockPenalty;
    }

    qInfo() << totalSecs << " Total secs ";

    uint64_t years = totalSecs / SECS_YEAR;
    totalSecs %= SECS_YEAR;

    uint64_t days = totalSecs / SECS_DAY;
    totalSecs %= SECS_DAY;

    uint64_t hours = totalSecs / SECS_HOURS;
    totalSecs %= SECS_HOURS;

    uint64_t minutes = totalSecs / SECS_MINUTES;
    totalSecs %= SECS_MINUTES;

    uint64_t secs = totalSecs;

    qInfo() << years << " year(s) - " << days << " day(s) - " << hours << " hour(s) - " << minutes
            << " minute(s)  " << secs << " sec(s). ";
}

void checkPswdStrength(std::string pswd)
{
    uint8_t bitMask = 0;

    for (unsigned char ch : pswd) {
        if ((ch >= '!' && ch <= '/') || (ch >= ':' && ch <= '@') || (ch >= '[' && ch <= '`')
            || (ch >= '{' && ch <= '~'))

            bitMask |= 1 << 0; // SPECIAL

        else if (ch >= '0' && ch <= '9')

            bitMask |= 1 << 1; // DIGITS

        else if (ch >= 'A' && ch <= 'Z')

            bitMask |= 1 << 2; // UPR_LAT_LETTERS

        else if (ch >= 'a' && ch <= 'z')

            bitMask |= 1 << 3; // LWR_LAT_LETTERS

        else if ((ch >= 128 && ch <= 159) || ch == 240)

            bitMask |= 1 << 4; // UPR_CYR_LETTERS

        else if ((ch >= 160 && ch <= 175) || (ch >= 224 && ch <= 239) || ch == 241)

            bitMask |= 1 << 5; // LWR_CYR_LETTERS
    }

    uint32_t alphPower = countAlphabetPower(bitMask);
    qInfo() << "Power of the alphabet: " << alphPower << "";
    qInfo() << "Password length: " << pswd.size() << "";

    uint64_t cmbnQnty;

    try {
        cmbnQnty = doBinExp(alphPower, static_cast<uint32_t>(pswd.size()));
    } catch (std::overflow_error) {
        qInfo() << "Combinations quantity limit for C++ uint64_t exceeded, combinations quantity "
                   "set to 2^64 ";
        cmbnQnty = UINT64_MAX_VALUE;
    }

    countHackTime(cmbnQnty, SECS_FOR_COMBINATION, ATTEMPTS_BEFORE_LOCK, SECS_FOR_UNLOCK);
}

void countHackTimePrecise(uint8_t alphabetBitMask, uint8_t passwordLength, double avgMsPerAttempt)
{
    const uint64_t MS_YEAR = 365ULL * 24 * 60 * 60 * 1000;
    const uint64_t MS_DAY = 24ULL * 60 * 60 * 1000;
    const uint64_t MS_HOUR = 60ULL * 60 * 1000;
    const uint64_t MS_MINUTE = 60ULL * 1000;

    uint32_t alphPower = countAlphabetPower(alphabetBitMask);

    uint64_t cmbnQnty;
    try {
        cmbnQnty = doBinExp(alphPower, passwordLength);
    } catch (std::overflow_error&) {
        qInfo() << "Overflow. Combinations quantity set to 2^64";
        cmbnQnty = UINT64_MAX_VALUE;
    }

    // Вычисляем общее время в миллисекундах как double, чтобы избежать потери точности
    double totalMs = avgMsPerAttempt * static_cast<double>(cmbnQnty);

    // Для вывода разбиваем время на компоненты, используя double для дробной точности
    double yearsD = totalMs / static_cast<double>(MS_YEAR);
    uint64_t years = static_cast<uint64_t>(yearsD);
    totalMs -= years * MS_YEAR;

    double daysD = totalMs / static_cast<double>(MS_DAY);
    uint64_t days = static_cast<uint64_t>(daysD);
    totalMs -= days * MS_DAY;

    double hoursD = totalMs / static_cast<double>(MS_HOUR);
    uint64_t hours = static_cast<uint64_t>(hoursD);
    totalMs -= hours * MS_HOUR;

    double minutesD = totalMs / static_cast<double>(MS_MINUTE);
    uint64_t minutes = static_cast<uint64_t>(minutesD);
    totalMs -= minutes * MS_MINUTE;

    double secondsD = totalMs / 1000.0;
    uint64_t seconds = static_cast<uint64_t>(secondsD);
    totalMs -= seconds * 1000.0;

    uint64_t milliseconds = static_cast<uint64_t>(totalMs);

    double fractionalMs = avgMsPerAttempt * cmbnQnty
                          - static_cast<uint64_t>(avgMsPerAttempt * cmbnQnty);

    qInfo().noquote() << QString("Alphabet bitmask: 0x%1 | Power: %2 | Length: %3")
                             .arg(alphabetBitMask, 0, 16)
                             .arg(alphPower)
                             .arg(passwordLength);
    qInfo().noquote() << QString("Average time per attempt (ms): %1")
                             .arg(avgMsPerAttempt, 0, 'f', 10);
    qInfo() << "Total time: " << years << " year(s) - " << days << " day(s) - " << hours
            << " hour(s) - " << minutes << " minute(s) - " << seconds << " second(s) - "
            << milliseconds << " ms" << QString(" + %1 fractional ms").arg(fractionalMs, 0, 'f', 6);
}

// Overload for Qt impl
void checkPswdStrength(const QString& pswd)
{
    uint8_t bitMask = 0;

    for (QChar ch : pswd) {
        ushort u = ch.unicode();

        if ((u >= 33 && u <= 47) || (u >= 58 && u <= 64) || (u >= 91 && u <= 96)
            || (u >= 123 && u <= 126))
            bitMask |= 1 << 0; // SPECIAL

        else if (u >= '0' && u <= '9')
            bitMask |= 1 << 1; // DIGITS

        else if (u >= 'A' && u <= 'Z')
            bitMask |= 1 << 2; // UPR_LAT_LETTERS

        else if (u >= 'a' && u <= 'z')
            bitMask |= 1 << 3; // LWR_LAT_LETTERS

        else if ((u >= 0x0410 && u <= 0x042F) || u == 0x0401) // А-Я, Ё
            bitMask |= 1 << 4;                                // UPR_CYR_LETTERS

        else if ((u >= 0x0430 && u <= 0x044F) || u == 0x0451) // а-я, ё
            bitMask |= 1 << 5;                                // LWR_CYR_LETTERS
    }

    uint32_t alphPower = countAlphabetPower(bitMask);
    qInfo() << "Power of the alphabet: " << alphPower;
    qInfo() << "Password length: " << pswd.length();

    uint64_t cmbnQnty;

    try {
        cmbnQnty = doBinExp(alphPower, static_cast<uint32_t>(pswd.length()));
    } catch (std::overflow_error&) {
        qInfo() << "Overflow. Combinations quantity set to 2^64";
        cmbnQnty = UINT64_MAX_VALUE;
    }

    countHackTime(cmbnQnty, SECS_FOR_COMBINATION, ATTEMPTS_BEFORE_LOCK, SECS_FOR_UNLOCK);
}

QString buildAlphabet(uint8_t bitMask)
{
    QString alphabet;

    if (bitMask & (1 << 0))
        alphabet += "0123456789";
    if (bitMask & (1 << 1))
        alphabet += "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (bitMask & (1 << 2))
        alphabet += "abcdefghijklmnopqrstuvwxyz";
    if (bitMask & (1 << 3))
        alphabet += "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ";
    if (bitMask & (1 << 4))
        alphabet += "абвгдеёжзийклмнопрстуфхцчшщъыьэюя";
    if (bitMask & (1 << 5))
        alphabet += "!@#$%^&*()-_=+[]{}|;:'\",.<>/?`~";

    return alphabet;
}

void hackPass(std::function<void(QString)> logInFunc,
              uint8_t alphabetBitMask,
              bool* stopFlag,
              uint8_t passLength)
{
    QString alpha = buildAlphabet(alphabetBitMask);
    uint8_t symbolsInAlphabet = alpha.length();
    uint64_t attempts = 0;
    std::vector<uint8_t> passIndexesCombinations(passLength, 0);

    // Func for alphabet combinations iteration
    auto iterateVector = [passLength, symbolsInAlphabet, stopFlag, &passIndexesCombinations]() {
        int64_t currPos = passLength - 1;
        while (currPos >= 0 && !(*stopFlag)) {
            if (passIndexesCombinations[currPos] + 1 < symbolsInAlphabet) {
                passIndexesCombinations[currPos]++;
                return true;
            } else {
                passIndexesCombinations[currPos] = 0;
                currPos--;
            }
        }
        return false;
    };

    QElapsedTimer timer;
    timer.start();

    while (true) {
        attempts++;
        QString passAttempt;
        for (uint8_t i : passIndexesCombinations)
            passAttempt.append(alpha[i]);

        if (attempts % 10000 == 0)
            qDebug() << "Hacking, attempt: " << attempts << "current password" << passAttempt;

        logInFunc(passAttempt);

        if (!iterateVector()) {
            qInfo() << "Password hacked: " << passAttempt;
            break;
        }
    }

    qint64 elapsedMs = timer.elapsed();
    double timePerAttempt = static_cast<double>(elapsedMs) / attempts;

    qInfo() << "Total attempts:" << attempts;
    qInfo() << "Total time (ms):" << elapsedMs;
    qInfo() << "Average time per attempt (ms):" << timePerAttempt;
    countHackTimePrecise(0x3F, passLength, timePerAttempt);
}

#endif // PASSANALYZER_H
