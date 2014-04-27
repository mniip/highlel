#pragma once

#include <stdexcept>

class SegmentationViolation: public std::exception
{
public:
	SegmentationViolation(): exception() { }
	virtual const char *what() const throw()
	{
		return "Segmentation fault";
	}
};

class InvalidInstruction: public std::exception
{
public:
	InvalidInstruction(): exception() { }
	virtual const char *what() const throw()
	{
		return "Illegal instruction";
	}
};
