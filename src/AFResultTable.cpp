///
/// \author John Farrier
/// \author Brian Kloppenborg
///
/// \copyright Copyright 2014 John Farrier, Brian Kloppenborg
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
/// http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///

#include "AFResultTable.h"
#include <celero/PimplImpl.h>
#include <celero/Benchmark.h>

using namespace celero;

#include <assert.h>

#include <map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>

///
/// \class Impl
///
class AFResultsTable::Impl
{
public:
    std::vector<std::string> staticColumnHeaders;
    std::vector<std::string> staticColumnValues;
    std::string fileName;
    std::map<std::string, std::map<std::string, std::vector<std::pair<int64_t, double>>>> results;

public:
        Impl() :
            fileName(),
            results()
        {
        }
};

AFResultsTable::AFResultsTable() : pimpl()
{
}

AFResultsTable::~AFResultsTable()
{
}

AFResultsTable& AFResultsTable::Instance()
{
    static AFResultsTable singleton;
    return singleton;
}

void AFResultsTable::setFileName(const std::string& x)
{
    assert(x.empty() == false);
    this->pimpl->fileName = x;
}

void AFResultsTable::addStaticColumn(const std::string& header, const std::string& value)
{
    assert(header.empty() == false);
    assert(value.empty() == false);
    this->pimpl->staticColumnHeaders.push_back(header);
    this->pimpl->staticColumnValues.push_back(value);
}

void AFResultsTable::add(std::shared_ptr<Result> x)
{
    const auto measurements = std::make_pair(x->getProblemSpaceValue(), x->getUsPerCall());
    this->pimpl->results[x->getExperiment()->getBenchmark()->getName()][x->getExperiment()->getName()].push_back(measurements);
    this->save();
}

void AFResultsTable::save()
{
    std::ofstream ofs;
    ofs.open(this->pimpl->fileName);

    const std::vector<std::string> staticHeaders = this->pimpl->staticColumnHeaders;
    const std::vector<std::string> staticValues = this->pimpl->staticColumnValues;

    if(ofs.is_open() == true)
    {
        const auto os = &ofs;

        std::for_each(std::begin(this->pimpl->results), std::end(this->pimpl->results),
            [&os, &staticHeaders, &staticValues](decltype(*std::begin(this->pimpl->results))& group)
            {
                *os << group.first << "\n";

                const auto run = group.second;

                std::for_each(std::begin(run), std::end(run),
                    [run, os, staticHeaders, staticValues](decltype(*std::begin(run))& result)
                    {
                        auto vec = result.second;

                        // Write the header row
                        if(result.first == std::begin(run)->first)
                        {
                            *os << ",";
                            for(auto header: staticHeaders)
                                *os << header << ",";

                            std::for_each(std::begin(vec), std::end(vec),
                                [os](decltype(*std::begin(vec))& element)
                                {
                                    *os << element.first << ",";
                                });
                            *os << "\n";
                        }

                        // Write the data row
                        *os << result.first << ",";
                        for(auto value: staticValues)
                            *os << value << ",";

                        std::for_each(std::begin(vec), std::end(vec),
                            [os](decltype(*std::begin(vec))& element)
                            {
                                *os << element.second << ",";
                            });
                        *os << "\n";
                    });

                *os << "\n";
            });

        ofs.close();
    }
}

