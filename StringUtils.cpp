#include "pch.h"

#include "StringUtils.h"

/* The DEBUG_COL macro is used to help debug issues with column
 * alignment and positioning.  If enabled, it will print, to stderr, a
 * string of characters representing what's being appended to the
 * stream in each column of text.  To use it, rename the first macro
 * with the cerr statement to DEBUG_COL and the second macro to
 * DEBUG_COL_NO.  To disable, do the opposite. */
#define DEBUG_COL_NO(LABEL,ADDED)                                       \
   if (ADDED > 0)                                                       \
      std::cerr << std::string(ADDED,LABEL) << std::flush;

#define DEBUG_COL(LABEL,ADDED)

namespace nzg
{
    namespace StringUtils
    {
        HexDumpDataConfig ::
            HexDumpDataConfig()
            : showIndex(true), hexIndex(true), upperHex(false),
            idxDigits(4), indexSep(": "), groupBy(1), groupSep(" "),
            group2By(8), group2Sep("  "), bytesPerLine(16),
            showText(true), preText("    "),
            showBaseData(false), showBaseIndex(false)
        {}


        HexDumpDataConfig ::
            HexDumpDataConfig(bool ashowIndex, bool ahexIndex, bool aupperHex,
                unsigned aidxDigits, unsigned aindexWS,
                unsigned agroupBy, unsigned agroupWS,
                unsigned agroup2By, unsigned agroup2WS,
                unsigned abytesPerLine, bool ashowText,
                char aseparator, unsigned atextWS,
                bool aShowBaseData, bool aShowBaseIndex)
            : showIndex(ashowIndex), hexIndex(ahexIndex),
            upperHex(aupperHex), idxDigits(aidxDigits),
            groupBy(agroupBy), group2By(agroup2By),
            bytesPerLine(abytesPerLine), showText(ashowText),
            showBaseData(aShowBaseData), showBaseIndex(aShowBaseIndex)
        {
            indexSep = ":" + std::string(aindexWS, ' ');
            groupSep = std::string(agroupWS, ' ');
            group2Sep = std::string(agroup2WS, ' ');
            preText = std::string(atextWS, ' ');
            if (aseparator != 0)
            {
                preText += aseparator;
                postText = std::string(1, aseparator);
            }
        }


        HexDumpDataConfig ::
            HexDumpDataConfig(bool ashowIndex, bool ahexIndex, bool aupperHex,
                unsigned aidxDigits, const std::string& aindexSep,
                unsigned agroupBy, const std::string& agroupSep,
                unsigned agroup2By, const std::string& agroup2Sep,
                unsigned abytesPerLine, bool ashowText,
                char aseparator, const std::string& atextSep,
                bool aShowBaseData, bool aShowBaseIndex,
                const std::string& adataEndSep,
                const std::string& adataFinal)
            : showIndex(ashowIndex), hexIndex(ahexIndex),
            upperHex(aupperHex), idxDigits(aidxDigits),
            groupBy(agroupBy), group2By(agroup2By),
            bytesPerLine(abytesPerLine), showText(ashowText),
            indexSep(aindexSep), groupSep(agroupSep), group2Sep(agroup2Sep),
            showBaseData(aShowBaseData), showBaseIndex(aShowBaseIndex),
            dataEndSep(adataEndSep), dataFinal(adataFinal)
        {
            preText = atextSep;
            if (aseparator != 0)
            {
                preText += aseparator;
                postText = std::string(1, aseparator);
            }
        }


        HexDumpDataConfig ::
            HexDumpDataConfig(bool ashowIndex, bool ahexIndex, bool aupperHex,
                unsigned aidxDigits, const std::string& aindexSep,
                unsigned agroupBy, const std::string& agroupSep,
                unsigned agroup2By, const std::string& agroup2Sep,
                unsigned abytesPerLine, bool ashowText,
                const std::string& apreText,
                const std::string& apostText,
                bool aShowBaseData, bool aShowBaseIndex,
                const std::string& adataEndSep,
                const std::string& adataFinal,
                const std::string& aprefix)
            : showIndex(ashowIndex), hexIndex(ahexIndex),
            upperHex(aupperHex), idxDigits(aidxDigits),
            groupBy(agroupBy), group2By(agroup2By),
            bytesPerLine(abytesPerLine), showText(ashowText),
            indexSep(aindexSep), groupSep(agroupSep), group2Sep(agroup2Sep),
            preText(apreText), postText(apostText),
            showBaseData(aShowBaseData), showBaseIndex(aShowBaseIndex),
            dataEndSep(adataEndSep), dataFinal(adataFinal), prefix(aprefix)
        {
        }


        unsigned HexDumpDataConfig ::
            computeLineSize(unsigned bytesThisLine,
                bool lastLine)
            const
        {
            unsigned linesize = prefix.length();
            unsigned w2 = 0;
            unsigned w1 = 0;
            if (this->showIndex)
            {
                // number of characters used by index
                linesize += this->idxDigits + this->indexSep.length();
                if (this->showBaseIndex)
                    linesize += 2; // "0x"
            }
            // 2 characters per byte for data
            linesize += bytesThisLine * 2;
            if (this->showBaseData)
            {
                // 2 characters per group 1 for base/radix "0x"
                linesize += (bytesThisLine / this->groupBy) * 2;
                // extra radix and groupSep for incomplete group
                if (bytesThisLine % this->groupBy)
                {
                    linesize += 2;
                    w1++;
                }
            }
            if (this->group2By)
            {
                w2 += ((bytesThisLine / this->group2By) -
                    // no group 2 separator at the end of the line
                    ((bytesThisLine % this->group2By) == 0 ? 1 : 0));
            }
            if (this->groupBy)
            {
                // number of group 1's minus the number of group 2
                // separators already computed and -1 for no separator
                // at the end of the line
                w1 += (bytesThisLine / this->groupBy) - w2 - 1;
            }
            if (this->groupBy > 0)
            {
                // characters of white space between level 1 grouped data
                linesize += this->groupSep.length() * w1;
            }
            if (this->group2By > 0)
            {
                // characters of white space between level 2 grouped data
                linesize += this->group2Sep.length() * w2;
            }
            if (lastLine)
                linesize += this->dataFinal.length();
            else
                linesize += this->dataEndSep.length();
            return linesize;
        }


        std::string HexDumpDataConfig ::
            baseIndex()
            const
        {
            if (showBaseIndex && hexIndex)
            {
                return (upperHex ? "0X" : "0x");
            }
            return "";
        }


        std::string HexDumpDataConfig ::
            baseData()
            const
        {
            if (showBaseData)
            {
                return (upperHex ? "0X" : "0x");
            }
            return "";
        }

        void hexDumpData(const std::string& data, std::ostream& s,
            const HexDumpDataConfig& cfg)
        {
            // save the state in a stream object that doesn't need an
            // externally defined buffer or any of that crap.
            std::ostringstream oldState;
            oldState.copyfmt(s);
            std::string ascii = "";
            int col = 0;
            int datasize = data.size();
            unsigned linesize;

            if (cfg.groupBy && ((cfg.bytesPerLine % cfg.groupBy) != 0))
            {
                s << "hexDumpData: cfg.bytesPerLine % cfg.groupBy != 0"
                    << std::endl;
                s.copyfmt(oldState);
                return;
            }
            if (cfg.group2By && ((cfg.bytesPerLine % cfg.group2By) != 0))
            {
                s << "hexDumpData: cfg.bytesPerLine % cfg.group2By != 0"
                    << std::endl;
                s.copyfmt(oldState);
                return;
            }
            if (cfg.groupBy && ((cfg.group2By % cfg.groupBy) != 0))
            {
                s << "hexDumpData: cfg.group2By % cfg.groupBy != 0"
                    << std::endl;
                s.copyfmt(oldState);
                return;
            }

            // line format:
            // <prefix><index><indexSep><group1byte1>...<group1byte[groupBy]><groupSep>...<group[group2By]byte1>...<group[group2By]byte[groupBy]><group2Sep>....<byte[bytesPerLine]><dataEndSep/dataFinal><preText><separator><text><separator>\n
            // make sure our default formatting options are set
            s << std::hex << std::internal << std::noshowbase << std::setw(0);

            unsigned bytesOnLastLine = datasize % cfg.bytesPerLine;
            if (bytesOnLastLine == 0)
                bytesOnLastLine = cfg.bytesPerLine;
            linesize = std::max(
                cfg.computeLineSize(cfg.bytesPerLine, false),
                cfg.computeLineSize(bytesOnLastLine, true));

            for (int i = 0; i < datasize; i++)
            {
                if (i % cfg.bytesPerLine == 0)
                {
                    // add the prefix text at the beginning of each line
                    s << cfg.prefix;
                    col = cfg.prefix.length();
                    DEBUG_COL('P', cfg.prefix.length());
                    if (cfg.showIndex)
                    {
                        // print the data index in either hex or decimal,
                        // with or without a radix indicator all
                        // according to cfg
                        std::string indexBase(cfg.baseIndex());
                        s << std::setfill('0')
                            << (cfg.upperHex ? std::uppercase : std::nouppercase)
                            << cfg.baseIndex()
                            << (cfg.hexIndex ? std::hex : std::dec)
                            << std::setw(cfg.idxDigits)
                            << i << cfg.indexSep
                            << std::noshowbase << std::dec << std::nouppercase;
                        col += indexBase.length() + cfg.idxDigits +
                            cfg.indexSep.length();
                        DEBUG_COL('I', indexBase.length() + cfg.idxDigits + cfg.indexSep.length());
                    }
                }
                unsigned char c = data[i];
                // construct the ASCII representation using only
                // printable characters
                if (isprint(c))
                    ascii += c;
                else
                    ascii += '.';
                if (cfg.groupBy && ((i % cfg.groupBy) == 0) && cfg.showBaseData)
                {
                    // print the hex radix indicator if requested
                    s << cfg.baseData();
                    col += 2;
                    DEBUG_COL('R', 2);
                }
                // print the byte value in hex
                s << (cfg.upperHex ? std::uppercase : std::nouppercase)
                    << std::hex << std::setw(2) << std::setfill('0') << (int)c
                    << std::dec << std::nouppercase << std::noshowbase;
                col += 2;
                DEBUG_COL('B', 2);
                if (((i % cfg.bytesPerLine) == (cfg.bytesPerLine - 1)) ||
                    (i == (datasize - 1)))
                {
                    if (i == (datasize - 1))
                    {
                        // this is the very last byte of data, print the
                        // final terminator text dataFinal
                        s << cfg.dataFinal;
                        col += cfg.dataFinal.length();
                        DEBUG_COL('F', cfg.dataFinal.length());
                    }
                    else if ((i % cfg.bytesPerLine) == (cfg.bytesPerLine - 1))
                    {
                        // this is the last byte on the line of text,
                        // print the end-of-line terminator text
                        // dataEndSep
                        s << cfg.dataEndSep;
                        col += cfg.dataEndSep.length();
                        DEBUG_COL('E', cfg.dataEndSep.length());
                    }
                    if (cfg.showText)
                    {
                        // print the ASCII representation of the data
                        int extra = linesize - col;
                        std::string space(extra, ' ');
                        s << space << cfg.preText;
                        DEBUG_COL('s', extra);
                        DEBUG_COL('T', cfg.preText.length());
                        s << ascii;
                        DEBUG_COL('A', ascii.length());
                        s << cfg.postText;
                        DEBUG_COL('t', cfg.postText.length());
                    }
                    s << std::endl;
                    DEBUG_COL('\n', 1);
                    ascii.erase();
                }
                else if (cfg.group2By && ((i % cfg.group2By) == (cfg.group2By - 1)))
                {
                    // level 2 group separator
                    s << cfg.group2Sep;
                    col += cfg.group2Sep.length();
                    DEBUG_COL('O', cfg.group2Sep.length());
                }
                else if (cfg.groupBy && ((i % cfg.groupBy) == (cfg.groupBy - 1)))
                {
                    // level 1 group separator
                    s << cfg.groupSep;
                    col += cfg.groupSep.length();
                    DEBUG_COL('G', cfg.groupSep.length());
                }
            }
            s.copyfmt(oldState);
        }


        std::string floatFormat(double d, FFLead lead, unsigned mantissa,
            unsigned exponent, unsigned width, char expChar,
            FFSign sign, FFAlign align)
        {
            std::ostringstream oss;
            std::string rv;
            std::string::size_type pos;
            switch (sign)
            {
            case FFSign::NegOnly:
                // default behavior for iostream
                break;
            case FFSign::NegSpace:
                if (d >= 0)
                    oss << " ";
                break;
            case FFSign::NegPos:
                oss << std::showpos;
                break;
            }
            switch (lead)
            {
            case FFLead::Zero:
                // because we're shifting the decimal point, we multiply by 10
                // mantissa-2 because we're adding a digit that
                // isn't present in std c++ iostream.
                oss << std::scientific << std::setprecision(mantissa - 2)
                    << (d * 10);
                rv = oss.str();
                // move the decimal
                pos = rv.find('.');
                rv[pos] = rv[pos - 1];
                rv[pos - 1] = '.';
                rv.insert(pos - 1, 1, '0');
                break;
            case FFLead::Decimal:
                // because we're shifting the decimal point, we multiply by 10
                oss << std::scientific << std::setprecision(mantissa - 1)
                    << (d * 10);
                rv = oss.str();
                // move the decimal
                pos = rv.find('.');
                rv[pos] = rv[pos - 1];
                rv[pos - 1] = '.';
                break;
            case FFLead::NonZero:
                // This is the default behavior for C++ iostream.
                oss << std::scientific << std::setprecision(mantissa - 1) << d;
                rv = oss.str();
                break;
            }
            // change the exponent size if needed
            pos = rv.find('e');
            unsigned currentExpSize = rv.length() - (pos + 2);
            if (currentExpSize < exponent)
                rv.insert(pos + 2, exponent - currentExpSize, '0');
            // change exponent character
            rv[pos] = expChar;
            // fill according to alignment request
            if (rv.length() < width)
            {
                switch (align)
                {
                case FFAlign::Left:
                    rv.append(width - rv.length(), ' ');
                    break;
                case FFAlign::Right:
                    rv.insert(0, width - rv.length(), ' ');
                    break;
                }
            }
            return rv;
        }
    } // namespace StringUtils
} // namespace gnsstk
