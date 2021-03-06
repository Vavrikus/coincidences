#include "TCanvas.h"
#include "TString.h"
#include "TChain.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TFile.h"
#include "TVector3.h"
#include "TMath.h"
#include "TSystem.h"
#include "TTimeStamp.h"
#include "TGraph.h"
#include "THStack.h"
#include "TStyle.h"

#include <iostream>
#include <fstream>
#include <map>
#include <cmath>

#define DEBUG() cout << "Current Line: " << __LINE__ << "\n";

using namespace std;

struct Event
{
	int m_seasonID, m_clusterID, m_runID, m_eventID, m_nHits, m_nHitsAfterCaus;
	int m_nHitsAfterTFilter, m_nStringsAfterCaus, m_nStringsAfterTFilter, m_nTrackHits;
	double m_energy, m_theta, m_phi, m_mcEnergy, m_mcTheta, m_mcPhi;
	double m_energySigma, m_thetaSigma, m_phiSigma, m_directionSigma;
	double m_chi2AfterCaus, m_chi2AfterTFilter, m_cascTime, m_likelihood, m_likelihoodHitOnly;
	double m_qTotal;
	double m_rightAscension, m_declination;
	TVector3 m_position;
	TVector3 m_mcPosition;
	TTimeStamp m_eventTime;
};

struct Coincidence
{
	int m_numOfEvents;
	int m_eventsBeforeFilter;
	vector<Event> m_events;

	Coincidence& copy()
	{
		Coincidence* c;
		c->m_numOfEvents = this->m_numOfEvents;
		c->m_eventsBeforeFilter = this->m_eventsBeforeFilter;
		c->m_events = this->m_events;

		return *c;
	}

	//returns angular distance of events number i and j in degrees
	double angDist(int i, int j) const
	{
	    TVector3 v1(0,0,1);
	    v1.SetTheta(TMath::Pi()/2.0+m_events[i].m_declination);
	    v1.SetPhi(m_events[i].m_rightAscension);

	    TVector3 v2(0,0,1);
	    v2.SetTheta(TMath::Pi()/2.0+m_events[j].m_declination);
	    v2.SetPhi(m_events[j].m_rightAscension);

	    return 180.0*v1.Angle(v2)/TMath::Pi();
	}
};

// std::vector<double> stringXPositions = {5.22,52.13,57.54,25.17,-29.84,-53.6,-42.32,0};
// std::vector<double> stringYPositions = {62.32,37.15,-13.92,-52.01,-52.36,-7.49,42.74,0};

double xPos[40] = {-13.76,32.14,45.06,5.13,-45.03,-76.21,-59.85,-14.47,-195.19,-164.79,-180.08,-227.51,-276.24,-279.59,-248.17,-222.70,-270.25,-228.58,-220.89,-261.89,-309.86,-337.48,-319.74,-282.27,65.85,108.73,113.87,74.19,25.1,-2.48,16.08,58.37,-163.91,-119.26,-113.90,-152.28,-202.59,-230.83,-213.25,-170.30};
double yPos[40] = {-211.35,-235.88,-285.45,-325.83,-319.82,-281.63,-231.37,-270.17,-340.62,-384.09,-435.13,-450.13,-424.31,-372.59,-337.03,-391.09,-37.36,-65.26,-117.78,-153.57,-146.26,-101.43,-55.24,-96.82,-435.47,-462.39,-514.68,-549.90,-544.25,-500.53,-453,-491.97,-628.26,-656.49,-707.52,-744.24,-738.58,-694.13,-645.06,-685.35};

//{{{}},{{},{}},{{},{},{}},{{},{},{},{},{}},{{},{},{},{},{},{},{}}};//
vector<vector<vector<int>>> ledMatrixRuns = {{{2,3,4,5,6,7,8,9,10,11,118,119,177,193,194,200,201,228,229,230,231,232,233,234,235,236,237,560,598}},{{},{}},{{7,117,412,429,443,459,474,490,505,520,548,564,579,595},{1,2,3,6,7,37,134,340,428,450,464,480,495,510,527,540,568,584,599,615,631,647,668},{35,36,117,120,131,151,412,429,443,459,474,489,504,519,520,547,575,591,607,623,644}},{{17,18,37,38,39,40,44,61,77,93,97,111,126,142,158,174,190,203,218,232,247,264,277,292,362,377,392,407,422,437,452,467,484,536,551,566,583,596,611,628,644,661,676,677,693},{8,41,54,56,60,61,77,92,107,123,138,154,169,184,201,215,231,245,260,276,306,375,391,406,421,436,451,466,481,498,553,571,586,603,616,631,648,663,679,694,709},{8,9,10,24,80,93,109,124,139,155,170,185,201,216,233,247,262,276,291,329,330,331,337,406,422,437,453,468,483,498,513,530,594,595,596,597,611,612,629,642,657,674,689,705,720,735},{13,23,36,51,67,82,100,116,131,146,162,179,193,208,222,237,251,268,283,350,367,384},{13,23,34,50,67,82,86,88,89,90,91,92,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,112,113,114,116,117,118,120,121,122,123,124,129,130,132,137,147,163,180,193,208,222,237,238,253,265,279,363,379}},{{3,19,32,42,51,52,62,71,82,92,102,122,145,156,165,180},{12,14,24,33,35,42,51,60,69,83,90,111,134,145,146,147,155,156,157,158,159,160,162,164},{9,13,14,17,132,143,153,164,165,167,169,172},{1,15,17,21,26,36,46,58,67,76,86,94,103,112,114},{2,12,17,19,23,24,26,36,44,55,63,73,82,89,98,106,117,131,143,151,160,166,168,175},{18,20,25,31,41,51,62,71,90,110,118,128,130,143,145,154,157,163,166,173,178,185,195,220,232,241,250,260,282,296,301,312,326,336,346,356,367,384,394,404,414,425,434,442,451},{7,10,12,16,17,22,30,40,49,58,67,76,84,93,102,105,113,115,129,131,143,144,149,152,159,165,169,174,177,207,219,228,237,245,254,264,277,281,290,301,312,322,332,342,359,369,380,389,398,407,417,426}}};

int unix1995 = 788918400;

map<TString,TH1F*> flux_hist;
map<TString,THStack*> flux_stack;
map<TString,TCanvas*> flux_canv;

int seasonID, clusterID, runID, eventID, nHits, nHitsAfterCaus, nHitsAfterTFilter, nStringsAfterCaus, nStringsAfterTFilter, nTrackHits;
double energy,theta,phi,mcEnergy,mcTheta,mcPhi;
double energySigma,thetaSigma,phiSigma,directionSigma;
double chi2AfterCaus, chi2AfterTFilter, cascTime, likelihood, likelihoodHitOnly, qTotal;
double rightAscension, declination;
TVector3* position = new TVector3();
TVector3* mcPosition = new TVector3();
TTimeStamp* eventTime = new TTimeStamp();

vector<Event> sortedEvents;
int nCoincidences = 0;

vector<Coincidence> coincidences;
vector<Coincidence> filteredCoincidences;

//getting data from global variables to objects
void ParseEvent(Event& ev)
{	
	ev.m_seasonID 				= seasonID;
	ev.m_clusterID 				= clusterID;
	ev.m_runID 					= runID;
	ev.m_eventID 				= eventID;
	ev.m_nHits 					= nHits;
	ev.m_nHitsAfterCaus 		= nHitsAfterCaus;
	ev.m_nStringsAfterCaus 		= nStringsAfterCaus;
	ev.m_chi2AfterCaus 			= chi2AfterCaus;
	ev.m_nHitsAfterTFilter 		= nHitsAfterTFilter;
	ev.m_nStringsAfterTFilter 	= nStringsAfterTFilter;
	ev.m_chi2AfterTFilter 		= chi2AfterTFilter;
	ev.m_energy 				= energy;
	ev.m_energySigma 			= energySigma;
	ev.m_theta 					= theta;
	ev.m_thetaSigma 			= thetaSigma;
	ev.m_phi 					= phi;
	ev.m_phiSigma 				= phiSigma;
	ev.m_directionSigma 		= directionSigma;
	ev.m_declination 			= declination;
	ev.m_rightAscension 		= rightAscension;
	ev.m_position 				= *position;
	ev.m_eventTime 				= *eventTime;
	ev.m_cascTime 				= cascTime;
	ev.m_mcEnergy 				= mcEnergy;
	ev.m_mcTheta 				= mcTheta;
	ev.m_mcPhi 					= mcPhi;
	ev.m_mcPosition 			= *mcPosition;
	ev.m_likelihood 			= likelihood;
	ev.m_likelihoodHitOnly 		= likelihoodHitOnly;
	ev.m_qTotal 				= qTotal;
	ev.m_nTrackHits 			= nTrackHits;
}

//operator overloading for printing
std::ostream& operator<<(std::ostream& stream, const Event& ev)
{
	stream << "     =====================================  EVENT INFO  =====================================\n";
	stream << "     Time Stamp:\n          ";
	ev.m_eventTime.Print();

	stream << "\n     IDs:\n     ";
    stream << "     eventID: " << ev.m_eventID << ", seasonID: " << ev.m_seasonID << ", clusterID: ";
    stream << ev.m_clusterID << ", runID: " << ev.m_runID << "\n\n";

    stream << "     Reconstructed variables:\n     ";
    stream << "     energy = " << ev.m_energy << " TeV, sigma = " << ev.m_energySigma << " TeV\n     ";
    stream << "     theta = " << ev.m_theta/TMath::Pi()*180 << ", sigma = " << ev.m_thetaSigma/TMath::Pi()*180;
    stream << ", phi = " << ev.m_phi/TMath::Pi()*180 << ", sigma = " << ev.m_phiSigma/TMath::Pi()*180;
    stream << ", direction sigma = " << ev.m_directionSigma << "\n     ";
    stream << "     Position (XYZ): " << ev.m_position.X();
    stream << " " << ev.m_position.Y() << " " << ev.m_position.Z() << "\n     ";
    stream << "     right ascension = " << ev.m_rightAscension/TMath::Pi()*180;
    stream << ", declination = " << ev.m_declination/TMath::Pi()*180 << "\n\n";

    stream << "     Reconstruction parameters:\n     ";
    stream << "     nHits = " << ev.m_nHits << ", nTrackHits = " << ev.m_nTrackHits << ", qTotal = ";
    stream << ev.m_qTotal << "\n     ";
    stream << "     likelihood = " << ev.m_likelihood << ", likelihoodHitOnly = " << ev.m_likelihoodHitOnly;
    stream << "\n          cascTime = " << ev.m_cascTime << "\n   ";

    stream << "     After TFilter:\n     ";
    stream << "     nHits = " << ev.m_nHitsAfterTFilter <<  ", nStrings = " << ev.m_nStringsAfterTFilter;
    stream << ", chi2 = " << ev.m_chi2AfterTFilter << "\n   ";

    stream << "     After Caus:\n     ";
    stream << "     nHits = " << ev.m_nHitsAfterCaus << ", nStrings = " << ev.m_nStringsAfterCaus;
    stream << ", chi2 = " << ev.m_chi2AfterCaus << "\n\n";

    stream << "     MC data:\n     ";
    stream << "     energy = " << ev.m_mcEnergy << " TeV, theta = " << ev.m_mcTheta/TMath::Pi()*180; 
    stream << ", phi = " << ev.m_mcPhi/TMath::Pi()*180 << "\n     ";
    stream << "     Position (XYZ): " << ev.m_mcPosition.X();
    stream << " " << ev.m_mcPosition.Y() << " " << ev.m_mcPosition.Z() << "\n";

    return stream;
}

std::ostream& operator<<(std::ostream& stream, const Coincidence& c)
{
	stream << "#######################################  COINCIDENCE  #######################################\n";
	stream << "Number of events: " << c.m_numOfEvents << "\n";
	stream << "Events before angle filter: " << c.m_eventsBeforeFilter << "\n\n";

	stream << "Time differences: ";

	for(int i = 1; i < c.m_numOfEvents; i++)
	{
		if(i!=1) cout << ", ";
		stream << c.m_events[i].m_eventTime.GetSec()-c.m_events[i-1].m_eventTime.GetSec(); 
	}

	stream << "\nMinimal angular distance: ";
	double minAngDist = c.angDist(0,1);

	for (int i = 0; i < c.m_numOfEvents; i++)
	{
		for(int j = i+1; j < c.m_numOfEvents; j++)
		{
			if(c.angDist(i,j) < minAngDist)
			{
				minAngDist = c.angDist(i,j);
			}
		}
	}

	stream << minAngDist << "\n\n";

	for(auto const& ev : c.m_events) stream << ev << "\n\n";

    return stream;
}

void DrawResults(int val)
{
	for(auto const& x : flux_hist)
	{
		// flux_canv[x.first] = new TCanvas(x.first,"CascadeFlux",800,600);
		// x.second->Draw();

		TString season = x.first(1,4);
		TString cluster = x.first(0,1);

		//if THStack with given key does not exist, create one, fill THStack
		if(flux_stack.find(season)!=flux_stack.end()) flux_stack[season]->Add(x.second);
		else
		{
			TString stack_name = "hs_cascFlux_y"+season(2,2);
			flux_stack[season] = new THStack(stack_name,"Cascade flux over time; Month; NoE [#] per 7.3 days");

			flux_stack[season]->Add(x.second);
		}

		if(flux_stack.find(cluster)!=flux_stack.end()) flux_stack[cluster]->Add(x.second);
		else
		{
			TString stack_name = "hs_cascFlux_c"+cluster;
			flux_stack[cluster] = new THStack(stack_name,"Cascade flux over time; Month; NoE [#] per 7.3 days");

			flux_stack[cluster]->Add(x.second);
		}		
	}

	for(auto const& x : flux_stack)
	{
		flux_canv[x.first] = new TCanvas("c_cascFlux_" + x.first,"CascadeFlux",800,600);
		x.second->Draw("nostack");

		x.second->GetXaxis()->SetTimeDisplay(1);
		x.second->GetXaxis()->SetTimeFormat("%m");
		x.second->Draw("nostack");
  		gPad->BuildLegend(0.75,0.75,0.95,0.95,"");	
	}
}

void SaveResults(int year, int cluster)
{
	TString outputFileName = Form("cascFlux_y%dc%d.root",year,cluster);
	TFile* outputFile = new TFile(outputFileName,"RECREATE");
	for(auto const& x : flux_hist) x.second->Write();
	for(auto const& x : flux_stack) x.second->Write();
	for(auto const& x : flux_canv) x.second->Write();
}

bool IsContained(TVector3* position, double distFromCluster = 0)
{
	if (TMath::Sqrt(TMath::Power(position->X(),2)+TMath::Power(position->Y(),2)) < 60+distFromCluster && TMath::Abs(position->Z()) < 265+distFromCluster)
		return true;
	else
		return false;
}

bool IsUncontained(TVector3* position, double near, double far)
{
	double horizontalDist = TMath::Sqrt(TMath::Power(position->X(),2)+TMath::Power(position->Y(),2));
	double verticalDist = TMath::Abs(position->Z());
	if ((horizontalDist < far && horizontalDist > near && verticalDist < 263) || (horizontalDist < far && verticalDist < 263+(far-60) && verticalDist > 263+(near-60)))
		return true;
	else
		return false;
}

bool IsLEDMatrixRun(int year, int cluster, int run)
{
	bool isLEDMatrixRun = false;
	for (int i = 0; i < ledMatrixRuns[year-16][cluster].size(); ++i)
	{
		if (run == ledMatrixRuns[year-16][cluster][i])
		{
			isLEDMatrixRun = true;
			break;
		}
	}
	return isLEDMatrixRun;
}

//returns number of leap years since 2016 (copied from transformations.h UTCtoUnix function)
int GetLeapYears(int season)
{
    int leapYears = std::floor((season - 2016) / 4);
    leapYears -= std::floor((season - 2000) / 100);
    leapYears += std::floor((season - 2000) / 400);

    return leapYears;
}

//returns unix (1995) time of 01/04/YYYY 00:00:00
int GetStartTime(int season)
{
	return 1459468800+(season-2016)*(365+GetLeapYears(season))*86400-unix1995;
}

//returns unix (1995) time of 31/03/YYYY+1 23:59:59
int GetEndTime(int season)
{
	return 1491004799+(season-2016)*(365+GetLeapYears(season))*86400-unix1995;
}

void Swap(vector<Event>& arr, int i, int j)
{
	Event temp = arr[i];
	arr[i] = arr[j];
	arr[j] = temp;	
}

//partition for quicksort algorhitm
int Partition(vector<Event>& arr, int high, int low)
{
	int firstBigger = low + 1;

	for (int i = low + 1; i <= high; i++)
	{
		if(arr[i].m_eventTime.GetSec() <= arr[low].m_eventTime.GetSec())
		{
			Swap(arr,i,firstBigger);
			firstBigger = firstBigger+1;
		}
	}

	Swap(arr,low,firstBigger-1);
	return firstBigger-1;
}

void QuickSort(vector<Event>& arr, int high = -100, int low = 0)
{
	if(high == -100) high = arr.size()-1;
	if(high > low)
	{
		int pivot = Partition(arr,high,low);
		QuickSort(arr,high,pivot+1);
		QuickSort(arr,pivot-1,low);
	}
}

//writes warning if two cascades are separated by smaller than selected amount of time
//also saves coincidences into a vector
void WarnIfCloser(const vector<Event>& arr, long int maxSec)
{
	if(arr.size() > 0)
	{
		long int previousTime = arr[0].m_eventTime.GetSec();

		Coincidence c;

		bool IsCoincidence = false;

		for(int i = 1; i < arr.size(); i++)
		{
			if(arr[i].m_eventTime.GetSec() - previousTime <= maxSec)
			{	
				if(!IsCoincidence)
				{
					nCoincidences++;

					IsCoincidence = true;

					c.m_numOfEvents = 2;
					c.m_eventsBeforeFilter = 2;
					c.m_events.clear();
					c.m_events.push_back(arr[i-1]);
					c.m_events.push_back(arr[i]);
				}

				else
				{
					c.m_numOfEvents++;
					c.m_eventsBeforeFilter++;
					c.m_events.push_back(arr[i]);
				}
			}

			else
			{
				if(IsCoincidence) coincidences.push_back(c);
				IsCoincidence = false;
			} 

			previousTime = arr[i].m_eventTime.GetSec();
		}
		if(IsCoincidence) coincidences.push_back(c);
	}

	cout << "\n\n";
	for(auto const& c : coincidences) cout << c;
}

void WarnLEDMatrixRun(int minCoinSize)
{
	cout << "Possible LED matrix runs detected (coincidence with ";
	cout << minCoinSize << " or more events):\n";

	bool noLEDRunsDetected = true;

	for(auto const& c : coincidences)
	{
		for(int season = 2016; season < 2021; season++)
		{
			for(int cluster = 0; cluster < 10; cluster++)
			{
				if(c.m_events[0].m_seasonID == season && c.m_events[0].m_clusterID == cluster && c.m_numOfEvents >= minCoinSize)
				{
					noLEDRunsDetected = false;
					cout << "seasonID: "   << c.m_events[0].m_seasonID;
					cout << " clusterID: " << c.m_events[0].m_clusterID;
					cout << " runID: "     << c.m_events[0].m_runID << "\n";
				}

			}
		}
	}

	if(noLEDRunsDetected) cout << "No runs detected." << endl;
}

//filter coincidences by angular distance in degrees
void FilterCoincidences(double maxAngDist)
{
	filteredCoincidences.clear();

	//filtering coincidences
	for(int c = 0; c < coincidences.size(); c++)
	{
		bool HasCloseEvents = false;

		for (int i = 0; i < coincidences[c].m_numOfEvents; i++)
		{
			for(int j = i+1; j < coincidences[c].m_numOfEvents; j++)
			{
				if(coincidences[c].angDist(i,j) < maxAngDist)
				{
					HasCloseEvents = true;
					break;
				}
			}

			if(HasCloseEvents) break;
		}

		if(HasCloseEvents) filteredCoincidences.push_back(coincidences[c]);
	}

	//filtering events in each coincidence
	for(int c = 0; c < filteredCoincidences.size(); c++)
	{
		vector<int> eventsToRemove;

		for(int i = 0; i < filteredCoincidences[c].m_events.size(); i++)
		{
			bool HasCloseEvent = false;

			for (int j = 0; j < filteredCoincidences[c].m_events.size(); j++)
			{
				if((i!=j) and (filteredCoincidences[c].angDist(i,j) < maxAngDist))
				{
					HasCloseEvent = true;
					break;
				}
			}

			if(!HasCloseEvent) eventsToRemove.push_back(i);
		}

		for(int i = eventsToRemove.size() - 1; i > -1; i--)
		{
			filteredCoincidences[c].m_events.erase(filteredCoincidences[c].m_events.begin()+eventsToRemove[i]);
			filteredCoincidences[c].m_numOfEvents--;
		}
	}

	cout << "\n\nFiltered coincidences with maximal distance " << maxAngDist << " degrees:\n";
	for(auto const& c : filteredCoincidences) cout << c;
}

int cascade_flux(int val = 0, int year = -1, int cluster = -1)
{
	int LCut;
	int maxTimeDiff;

	cout << "Contained + 40 cut is applied.\n";
	cout << "Apply likelihood cut? [1/0]\n";
	cin >> LCut;

	bool DoLikelihoodCut = LCut == 1;
	if(DoLikelihoodCut) cout << "Likelihood (Lho <= 1.5) cut is applied.\n";

	cout << "\nMaximal time difference for coincidences in seconds: \n";
	cin >> maxTimeDiff;
	cout << "Maximal time difference set to " << maxTimeDiff << " seconds.\n";

	TChain reconstructedCascades("Tree/t_RecCasc");

	TString filesDir;

	//choosing data based on cluster and year
	int startID = cluster!=-1?cluster:0;
	int endID = cluster!=-1?cluster+1:10;

	int startSeason = year!=-1?year:16;
	int endSeason = year!=-1?year+1:20+1;

	//path to data folder
	const char* env_p = val==1?"/home/vavrik/bajkal/recoCascades/v1.2":"/home/vavrik/work/data";

	for (int j = startSeason; j < endSeason; j++)
	{
		for (int i = startID; i < endID; ++i)
		{
			filesDir = Form("%s/exp20%d/cluster%d/",env_p,j,i);
			cout << filesDir << endl;

			auto dir = gSystem->OpenDirectory(filesDir.Data());
			while (auto f = gSystem->GetDirEntry(dir))
			{
			  	if (!strcmp(f,".") || !strcmp(f,"..")) continue;
			  	TString fullFilePath = filesDir + f + "/recCascResults.root";
			  	if (!gSystem->AccessPathName(fullFilePath))
			  	{
			  		// cout << f << endl;
			  		reconstructedCascades.Add(TString(filesDir) + f + "/recCascResults.root");
			  	}
			}
			gSystem->FreeDirectory(dir);
		}
	}

	TTree* filteredCascades = new TTree("filteredCascades","Filtered Cascades");

	reconstructedCascades.SetBranchAddress("seasonID", &seasonID);
	reconstructedCascades.SetBranchAddress("clusterID", &clusterID);
	reconstructedCascades.SetBranchAddress("runID", &runID);
	reconstructedCascades.SetBranchAddress("eventID", &eventID);
	reconstructedCascades.SetBranchAddress("nHits", &nHits);
	reconstructedCascades.SetBranchAddress("nHitsAfterCaus", &nHitsAfterCaus);
	reconstructedCascades.SetBranchAddress("nStringsAfterCaus", &nStringsAfterCaus);
	reconstructedCascades.SetBranchAddress("chi2AfterCaus", &chi2AfterCaus);
	reconstructedCascades.SetBranchAddress("nHitsAfterTFilter", &nHitsAfterTFilter);
	reconstructedCascades.SetBranchAddress("nStringsAfterTFilter", &nStringsAfterTFilter);
	reconstructedCascades.SetBranchAddress("chi2AfterTFilter", &chi2AfterTFilter);
	reconstructedCascades.SetBranchAddress("energy", &energy);
	reconstructedCascades.SetBranchAddress("energySigma", &energySigma);
	reconstructedCascades.SetBranchAddress("theta", &theta);
	reconstructedCascades.SetBranchAddress("thetaSigma", &thetaSigma);
	reconstructedCascades.SetBranchAddress("phi", &phi);
	reconstructedCascades.SetBranchAddress("phiSigma", &phiSigma);
	reconstructedCascades.SetBranchAddress("directionSigma", &directionSigma);
	reconstructedCascades.SetBranchAddress("declination",&declination);
	reconstructedCascades.SetBranchAddress("rightAscension",&rightAscension);
	reconstructedCascades.SetBranchAddress("position", &position);
	reconstructedCascades.SetBranchAddress("eventTime",&eventTime);
	reconstructedCascades.SetBranchAddress("time", &cascTime);
	reconstructedCascades.SetBranchAddress("mcEnergy", &mcEnergy);
	reconstructedCascades.SetBranchAddress("mcTheta", &mcTheta);
	reconstructedCascades.SetBranchAddress("mcPhi", &mcPhi);
	reconstructedCascades.SetBranchAddress("mcPosition", &mcPosition);
	reconstructedCascades.SetBranchAddress("likelihood", &likelihood);
	reconstructedCascades.SetBranchAddress("likelihoodHitOnly", &likelihoodHitOnly);
	reconstructedCascades.SetBranchAddress("qTotal", &qTotal);
	reconstructedCascades.SetBranchAddress("nTrackHits", &nTrackHits);

	filteredCascades->Branch("seasonID", &seasonID);
	filteredCascades->Branch("clusterID", &clusterID);
	filteredCascades->Branch("runID", &runID);
	filteredCascades->Branch("eventID", &eventID);
	filteredCascades->Branch("nHits", &nHits);
	filteredCascades->Branch("nHitsAfterCaus", &nHitsAfterCaus);
	filteredCascades->Branch("nStringsAfterCaus", &nStringsAfterCaus);
	filteredCascades->Branch("chi2AfterCaus", &chi2AfterCaus);
	filteredCascades->Branch("nHitsAfterTFilter", &nHitsAfterTFilter);
	filteredCascades->Branch("nStringsAfterTFilter", &nStringsAfterTFilter);
	filteredCascades->Branch("chi2AfterTFilter", &chi2AfterTFilter);
	filteredCascades->Branch("energy", &energy);
	filteredCascades->Branch("energySigma", &energySigma);
	filteredCascades->Branch("theta", &theta);
	filteredCascades->Branch("thetaSigma", &thetaSigma);
	filteredCascades->Branch("phi", &phi);
	filteredCascades->Branch("phiSigma", &phiSigma);
	filteredCascades->Branch("directionSigma", &directionSigma);
	filteredCascades->Branch("declination",&declination);
	filteredCascades->Branch("rightAscension",&rightAscension);
	filteredCascades->Branch("position", &position);
	filteredCascades->Branch("eventTime","TTimeStamp",&eventTime);
	filteredCascades->Branch("time", &cascTime);
	filteredCascades->Branch("mcEnergy", &mcEnergy);
	filteredCascades->Branch("mcTheta", &mcTheta);
	filteredCascades->Branch("mcPhi", &mcPhi);
	filteredCascades->Branch("mcPosition", &mcPosition);
	filteredCascades->Branch("likelihood", &likelihood);
	filteredCascades->Branch("likelihoodHitOnly", &likelihoodHitOnly);
	filteredCascades->Branch("qTotal", &qTotal);
	filteredCascades->Branch("nTrackHits", &nTrackHits);


	int nRecCasc = reconstructedCascades.GetEntries();

	cout << nRecCasc << endl;

	int nProcessedEvents = 0;

	sortedEvents.reserve(nRecCasc);

	for (int i = 0; i < reconstructedCascades.GetEntries(); ++i)
	{
		if(i%10000 == 0) cout << "Filtering cascades progress: " << (100.0*i)/nRecCasc << "%" << endl;
		reconstructedCascades.GetEntry(i);

		//remove cascades from calibration
		if (IsLEDMatrixRun(seasonID-2000,clusterID,runID))
			continue;

		//selecting only contained cascades with likelihoodHitOnly <= 1.5
		if (!IsContained(position,40) || (DoLikelihoodCut && (likelihoodHitOnly > 1.5))) //|| theta/TMath::Pi()*180 > 80)
		// if (!IsContained(position) || likelihoodHitOnly > 3)
		// if (!IsContained(position,40) || likelihoodHitOnly > 3 || nHitsAfterTFilter < 50)
		// if (!IsContained(position,40) || likelihoodHitOnly > 1.5 || position->Z() > 200)
		// if (!IsUncontained(position,60,100) || likelihoodHitOnly > 3)
			continue;

		nProcessedEvents++;
		filteredCascades->Fill();

		//key for histogram identification
		TString hist_key = to_string(clusterID)+to_string(seasonID);

		//event before 01/01/2016 warning
		if(eventTime->GetSec() < 1451606400)
			cout << "Event " << eventID << " has low eventTime: " << *eventTime << " seasonID: " << seasonID << " clusterID: " << clusterID << " runID: " << runID << "\n";

		//if histogram with given key does not exist, create one, fill histogram
		if(flux_hist.find(hist_key)!=flux_hist.end())
		{
			flux_hist[hist_key]->Fill(eventTime->GetSec()-unix1995-GetStartTime(seasonID)+GetStartTime(2016)); //1970 unix to 1995 unix
		}

		else
		{
			TString hist_name = Form("h_cascFlux_y%dc%d",seasonID-2000,clusterID);
			flux_hist[hist_key] = new TH1F(hist_name,hist_name(11,5)+"; Month; NoE [#]",50,GetStartTime(2016),GetEndTime(2016));

			flux_hist[hist_key]->GetXaxis()->SetTimeDisplay(1);
			flux_hist[hist_key]->GetXaxis()->SetTimeFormat("%m");//("%m/%Y");

			int color = seasonID-2014+clusterID;
			if(color > 9) color += 30;

			flux_hist[hist_key]->SetLineColor(color);	

			flux_hist[hist_key]->Fill(eventTime->GetSec()-unix1995-GetStartTime(seasonID)+GetStartTime(2016)); //1970 unix to 1995 unix

			cout << "Year: " << seasonID << " Cluster: " << clusterID << "\n";
		}

		Event ev;
		ParseEvent(ev);
		sortedEvents.push_back(ev);
	}
	
	QuickSort(sortedEvents);
	WarnIfCloser(sortedEvents,maxTimeDiff);
	FilterCoincidences(20);
	int nCoincidences2 = filteredCoincidences.size();
	FilterCoincidences(10);
	int nCoincidences3 = filteredCoincidences.size();
	WarnLEDMatrixRun(15);
	cout << "\nnCoincidences: " << nCoincidences << "\n";
	cout << "nCoincidences2: " << nCoincidences2 << "\n";
	cout << "nCoincidences3: " << nCoincidences3 << "\n";

	gStyle->SetOptStat(111111);

	DrawResults(val);
	SaveResults(year,cluster);

	cout << nProcessedEvents << endl;
	TString outputFileName = Form("filteredCascades_y%dc%d.root",year,cluster);
	TFile *newFile = new TFile(outputFileName,"recreate");
	filteredCascades->Write();
	newFile->Close();

	//for(auto ev : sortedEvents) cout << ev << "\n";

	return 0;
}

int main(int argc, char** argv) 
{
	int val, year, cluster;

	if(argc < 4) cluster = -1;
	else cluster = stoi(argv[3]);

	if(argc < 3) year = -1;
	else year = stoi(argv[2]);
	
	if(argc < 2) val = 0;
	else val = stoi(argv[1]);

	return cascade_flux(val,year,cluster);
}