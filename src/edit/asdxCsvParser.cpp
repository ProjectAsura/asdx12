//-----------------------------------------------------------------------------
// File : asdxCsvParser.cpp
// Desc : CSV Parser.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fstream>
#include <edit/asdxCsvParser.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// CsvParser class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      CSVファイルをパースします.
//-----------------------------------------------------------------------------
CsvParser::DataTable CsvParser::Parse(const char* path)
{
    DataTable   data;
    Row         row;
    std::string field;

    // CSV parser states
    enum class State
    {
        FieldStart,
        UnquotedField,
        QuotedField,
        AfterQuote
    };

    State state = State::FieldStart;

    std::ifstream stream(path);

    if (!stream)
        return data;

    if (stream.eof())
        return data;

    char ch;
    bool hasData = false;

    while (stream.get(ch))
    {
        hasData = true;

        switch (state)
        {

        case State::FieldStart:
            if (ch == ',')
            {
                row.push_back("");
            }
            else if (ch == '"')
            {
                state = State::QuotedField;
            }
            else if (ch == '\n')
            {
                row.push_back("");
                data.push_back(std::move(row));
                row.clear();
            }
            else if (ch == '\r')
            {
                // CRLF or CR
                if (stream.peek() == '\n')
                {
                    stream.get();
                }
                row.push_back("");
                data.push_back(std::move(row));
                row.clear();
            }
            else
            {
                field += ch;
                state = State::UnquotedField;
            }
            break;

        case State::UnquotedField:
            if (ch == ',')
            {
                row.push_back(std::move(field));
                field.clear();
                state = State::FieldStart;
            }
            else if (ch == '\n' || ch == '\r')
            {
                if (ch == '\r' && stream.peek() == '\n')
                {
                    stream.get();
                }

                row.push_back(std::move(field));
                field.clear();

                data.push_back(std::move(row));
                row.clear();

                state = State::FieldStart;
            }
            else if (ch == '"')
            {
                throw std::runtime_error(
                    "Unexpected quote in unquoted field"
                );
            }
            else
            {
                field += ch;
            }
            break;

        case State::QuotedField:
            if (ch == '"')
            {
                if (stream.peek() == '"')
                {
                    stream.get();
                    field += '"';
                }
                else
                {
                    state = State::AfterQuote;
                }
            }
            else
            {
                field += ch;
            }
            break;

        case State::AfterQuote:
            if (ch == ',')
            {
                row.push_back(std::move(field));
                field.clear();
                state = State::FieldStart;
            }
            else if (ch == '\n' || ch == '\r')
            {
                if (ch == '\r' && stream.peek() == '\n')
                {
                    stream.get();
                }

                row.push_back(std::move(field));
                field.clear();

                data.push_back(std::move(row));
                row.clear();

                state = State::FieldStart;
            }
            else
            {
                throw std::runtime_error("Unexpected character after closing quote");
            }
            break;
        }
    }

    // Check EOF
    if (state == State::QuotedField)
    {
        throw std::runtime_error("Unclosed quoted field");
    }

    // Add last row
    if (state != State::FieldStart || !row.empty() || hasData)
    {
        row .push_back(std::move(field));
        data.push_back(std::move(row));
    }

    return data;
}

} // namespace asdx
