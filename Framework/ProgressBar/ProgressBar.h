#pragma once
class ProgressBar
{
public:
	ProgressBar();
	~ProgressBar();

	ProgressBar(const ProgressBar&) = delete;
	ProgressBar& operator=(const ProgressBar&) = delete;

	void Begin();
	void Render();

	void Read() ;
private:
	std::string title;
	float xMin;
	float xMax;
	float yMin;
	float yMax;
	float height;
	
	bool bVisible;
	string status;
	float progress;
	
};

