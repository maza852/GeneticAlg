#include "INeiron.h"
#include "Internal_Neiron.h"
#pragma once

class Out_Neiron : public INeiron
{
public:
	float sum();
	float get_value();
	float activation();
	Out_Neiron(void);
	~Out_Neiron(void);
};

