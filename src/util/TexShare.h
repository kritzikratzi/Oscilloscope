//
//  SharedTex.hpp
//  Oscilloscope
//
//  Created by Hansi on 27/06/19.
//
//

#ifndef SharedTex_hpp
#define SharedTex_hpp

#include <memory>
#include <string>
class ofTexture;

class TexShareImpl;
class TexShare{
public:
	TexShare();
	~TexShare(); 
	void setup(std::string name);
	void update(ofTexture & tex);
	
private:
	std::unique_ptr<TexShareImpl> impl;
};


#endif /* SharedTex_hpp */
