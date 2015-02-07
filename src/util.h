//
//  util.h
//  audioOutputExample
//
//  Created by Hansi on 14.09.14.
//
//

#pragma once

template <typename valueType>
static std::vector <valueType> Split (valueType text, const valueType& delimiter)
{
    std::vector <valueType> tokens;
    size_t pos = 0;
    valueType token;
	
    while ((pos = text.find(delimiter)) != valueType::npos)
    {
        token = text.substr(0, pos);
        tokens.push_back (token);
        text.erase(0, pos + delimiter.length());
    }
    tokens.push_back (text);
	
    return tokens;
}