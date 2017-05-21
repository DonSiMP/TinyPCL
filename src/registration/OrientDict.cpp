
// Copyright (c) 2016-2017 Geosim Ltd.
// 
// Written by Amit Henig
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//


/******************************************************************************
*
*: Package Name: sldrcr_sp
*
******************************************************************************/

//#define DEBUG_LOCAL_RANGE_IMAGE
#ifdef DEBUG_LOCAL_RANGE_IMAGE
#include "dbg.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



namespace tpcl
{
  /******************************************************************************
  *
  *: Class name: SLDR_RDI_COrientedGrid
  *
  ******************************************************************************/


  /******************************************************************************
  *                             INTERNAL CONSTANTS  / Functions                 *
  ******************************************************************************/

  /******************************************************************************
  *                        INCOMPLETE CLASS DECLARATIONS                        *
  ******************************************************************************/

  /******************************************************************************
  *                       FORWARD FUNCTION DECLARATIONS                         *
  ******************************************************************************/

  /******************************************************************************
  *                             STATIC VARIABLES                                *
  ******************************************************************************/

  /******************************************************************************
  *                      CLASS STATIC MEMBERS INITIALIZATION                    *
  ******************************************************************************/

  /******************************************************************************
  *                              INTERNAL CLASSES                               *
  ******************************************************************************/




  /******************************************************************************
  *                           EXPORTED CLASS METHODS                            *
  ******************************************************************************/
  ///////////////////////////////////////////////////////////////////////////////
  //
  //                           COrientedGrid
  //
  ///////////////////////////////////////////////////////////////////////////////
  /******************************************************************************
  *                               Public methods                                *
  ******************************************************************************/
  /******************************************************************************
  *
  *: Method name: SLDRCR_RDI_COrientedGrid
  *
  ******************************************************************************/
  COrientedGrid::COrientedGrid()
  {
    initMembers();
  }


  COrientedGrid::COrientedGrid(float Xi_voxelSize)
  {
    initMembers();
    m_voxelSize = Xi_voxelSize;
    m_mainHashed = new CSpatialHash2D(m_voxelSize);
    ((CSpatialHash2D*)(m_mainHashed))->Clear();
  }


  /******************************************************************************
  *
  *: Method name: ~SLDRCR_RDI_COrientedGrid
  *
  ******************************************************************************/
  COrientedGrid::~COrientedGrid()
  {
    DeleteGrid();
  }



  /******************************************************************************
  *
  *: Method name: getVoxelSize
  *
  ******************************************************************************/
  float COrientedGrid::getVoxelSize()
  {
    return m_voxelSize;
  }


  /******************************************************************************
  *
  *: Method name: getMainHashedPtr
  *
  ******************************************************************************/
  void* COrientedGrid::getMainHashedPtr()
  {
    return m_mainHashed;
  }


  /******************************************************************************
  *
  *: Method name: getPtsMainPtr
  *
  ******************************************************************************/
  int COrientedGrid::getPtsMainPtr(CVec3* &Xo_ptsMain)
  {
    Xo_ptsMain = m_ptsMain;
    return m_numPtsMain;
  }

  /******************************************************************************
  *
  *: Method name: getGrid
  *
  ******************************************************************************/
  int COrientedGrid::getGrid(CMat4* &Xo_Orient)
  {
    Xo_Orient = m_Orient;
    return m_size;
  }


  /******************************************************************************
  *
  *: Method name: getBBox
  *
  ******************************************************************************/
  void COrientedGrid::getBBox(CVec3 &Xo_minBBox, CVec3 &Xo_maxBBox)
  {
    Xo_minBBox = m_minBBox;
    Xo_maxBBox = m_maxBBox;
  }



  /******************************************************************************
  *
  *: Method name: DeleteGrid
  *
  ******************************************************************************/
  void COrientedGrid::DeleteGrid()
  {
    delete m_mainHashed;
    delete[] m_ptsMain;
    delete[] m_Orient;
  }


  /******************************************************************************
  *
  *: Method name: ResetGrid
  *
  ******************************************************************************/
  void COrientedGrid::ResetGrid()
  {
    DeleteGrid();

    m_mainHashed = new CSpatialHash2D(m_voxelSize);
    ((CSpatialHash2D*)(m_mainHashed))->Clear();

    m_ptsMain = NULL;
    m_Orient = NULL;

    m_numPtsMain = 0;
    m_size = 0;
    m_minBBox = CVec3(0, 0, 0);
    m_maxBBox = CVec3(0, 0, 0);
  }


  /******************************************************************************
  *
  *: Method name: DeleteAndSetVoxelSize
  *
  ******************************************************************************/
  void COrientedGrid::DeleteAndSetVoxelSize(float Xi_voxelSize)
  {
    m_voxelSize = Xi_voxelSize;
    ResetGrid();
  }




  /******************************************************************************
  *
  *: Method name: PointCloudUpdate
  *
  ******************************************************************************/
  void COrientedGrid::PointCloudUpdate(int Xi_numPts, const CVec3* Xi_pts, CVec3& Xo_minBox, CVec3& Xo_maxBox)
  {
    PROFILE("PointCloudUpdate");

    CSpatialHash2D& mainHashed = *((CSpatialHash2D*)(m_mainHashed));

    int totalPts = m_numPtsMain + Xi_numPts;
    Features::resizeArray(m_ptsMain, m_numPtsMain, totalPts);

    Xo_minBox = Xo_maxBox = Xi_pts[0];
    for (int ptrIndex = 0; ptrIndex < Xi_numPts; ptrIndex++)
    {
      m_ptsMain[m_numPtsMain] = Xi_pts[ptrIndex];
      m_numPtsMain++;

      mainHashed.Add(Xi_pts[ptrIndex], (void*)(1));
      Xo_minBox = Min_ps(Xo_minBox, Xi_pts[ptrIndex]);
      Xo_maxBox = Max_ps(Xo_maxBox, Xi_pts[ptrIndex]);
    }
    //at end of for loop: m_numPtsMain = totalPts;

    m_minBBox = Min_ps(Xo_minBox, m_minBBox);;
    m_maxBBox = Max_ps(Xo_maxBox, m_maxBBox);;
  }




  /******************************************************************************
  *
  *: Method name: ViewpointGridUpdate
  *
  ******************************************************************************/
  int COrientedGrid::ViewpointGridUpdate(float Xi_d_grid, float Xi_d_sensor, CVec3& Xi_minBox, CVec3& Xi_maxBox)
  {
    PROFILE("ViewpointGridUpdate");

    CSpatialHash2D& mainHashed = *((CSpatialHash2D*)(m_mainHashed));

    float invGridRes = 1.0f / Xi_d_grid;

    //create grid's locations (taking z value from the closest point):
    int GridWidth = int(ceil((Xi_maxBox.x - Xi_minBox.x) * invGridRes));
    int GridHeight = int(ceil((Xi_maxBox.y - Xi_minBox.y) * invGridRes));
    int totalGridPoint = GridHeight*GridWidth;

    CVec3* gridPositions = new CVec3[totalGridPoint];

    #pragma omp parallel for
    for (int yGrid = 0; yGrid < GridHeight; yGrid++)
    {
      for (int xGrid = 0; xGrid < GridWidth; xGrid++)
      {
        CVec3 pos(Xi_minBox.x + xGrid*Xi_d_grid, Xi_minBox.y + yGrid*Xi_d_grid, 0);
        CVec3 closest;
        if (mainHashed.FindNearest(pos, &closest, Xi_d_grid))
          pos.z = closest.z;
        else
          pos.z = Xi_minBox.z;
        gridPositions[yGrid*GridWidth + xGrid] = pos;
      }
    }

    //find normals:
    const float maxDistForPlane = m_voxelSize * 2;
    CVec3* Normals = new CVec3[totalGridPoint];
    //TODO:      Features::FillPointCloud() \ outside source for ground?;
    Features::FindNormal(totalGridPoint, gridPositions, Normals, maxDistForPlane, &mainHashed, true);

    int preSize = m_size;
    m_size += totalGridPoint;
    Features::resizeArray(m_Orient, preSize, m_size);

    //creating final grid's transformation matrix:
#pragma omp parallel for //private(xGrid) collapse(2)
    for (int index = 0; index < totalGridPoint; index++)
    {
      //set viewpoints height above ground (in the normal vector direction above the plane that was found):
      gridPositions[index] = gridPositions[index] + (Xi_d_sensor * Normals[index]);

      //find refFrame matrix:
      //zVec = Normals[index]
      CVec3 xVec(1, 0, 0);
      xVec = xVec - DotProd(xVec, Normals[index])*Normals[index];
      Normalize(xVec);
      CVec3 yVec = CrossProd(Normals[index], xVec);
      Normalize(yVec);

      //update transformation matrix:
      float OrientVals[] = { xVec.x, yVec.x, Normals[index].x, 0.0f,
        xVec.y, yVec.y, Normals[index].y, 0.0f,
        xVec.z, yVec.z, Normals[index].z, 0.0f,
        gridPositions[index].x, gridPositions[index].y, gridPositions[index].z, 1.0f };
      m_Orient[preSize + index] = CMat4(OrientVals);
    }

    //release memory:
    delete[] gridPositions;
    delete[] Normals;

    return preSize;
  }



  /******************************************************************************
  *
  *: Method name: PointCloudAndGridUpdate
  *
  ******************************************************************************/
  int COrientedGrid::PointCloudAndGridUpdate(int Xi_numPts, const CVec3* Xi_pts, float Xi_d_grid, float Xi_d_sensor)
  {
    CVec3 minBox, maxBox;

    PointCloudUpdate(Xi_numPts, Xi_pts, minBox, maxBox);

    return ViewpointGridUpdate(Xi_d_grid, Xi_d_sensor, minBox, maxBox);
  }



  /******************************************************************************
  *                             Protected methods                               *
  ******************************************************************************/
  void COrientedGrid::initMembers()
  {
    m_numPtsMain = 0;
    m_ptsMain = NULL;
    m_voxelSize = 0.5;
    m_Orient = NULL;
    m_size = 0;
    m_mainHashed = NULL;
    m_minBBox = CVec3(0, 0, 0);
    m_maxBBox = CVec3(0, 0, 0);
  }

  /******************************************************************************
  *                              Private methods                                *
  ******************************************************************************/

  /******************************************************************************
  *                            EXPORTED FUNCTIONS                               *
  ******************************************************************************/

  /******************************************************************************
  *                            INTERNAL FUNCTIONS                               *
  ******************************************************************************/





  /***************************************************************************************************************************/




  /******************************************************************************
  *
  *: Class name: SLDR_RDI_CRegDictionary
  *
  ******************************************************************************/


  /******************************************************************************
  *                             INTERNAL CONSTANTS  / Functions                 *
  ******************************************************************************/

  /******************************************************************************
  *                        INCOMPLETE CLASS DECLARATIONS                        *
  ******************************************************************************/

  /******************************************************************************
  *                       FORWARD FUNCTION DECLARATIONS                         *
  ******************************************************************************/

  /******************************************************************************
  *                             STATIC VARIABLES                                *
  ******************************************************************************/

  /******************************************************************************
  *                      CLASS STATIC MEMBERS INITIALIZATION                    *
  ******************************************************************************/

  /******************************************************************************
  *                              INTERNAL CLASSES                               *
  ******************************************************************************/




  /******************************************************************************
  *                           EXPORTED CLASS METHODS                            *
  ******************************************************************************/
  ///////////////////////////////////////////////////////////////////////////////
  //
  //                           CRegDictionary
  //
  ///////////////////////////////////////////////////////////////////////////////
  /******************************************************************************
  *                               Public methods                                *
  ******************************************************************************/
  /******************************************************************************
  *
  *: Method name: SLDRCR_RDI_CRegDictionary
  *
  ******************************************************************************/
  /** constructor */
  CRegDictionary::CRegDictionary()
  {
    m_descriptors = NULL;
    m_descriptorsDFT = NULL;

    m_r_max = m_r_min = 0;
    m_descWidth = m_descHeight = 1;
  }


  CRegDictionary::CRegDictionary(float Xi_voxelSize, float Xi_r_max, float Xi_r_min, int Xi_descWidth, int Xi_descHeight) : COrientedGrid(Xi_voxelSize)
  {
    m_descriptors = NULL;
    m_descriptorsDFT = NULL;

    m_r_max = Xi_r_max;
    m_r_min = Xi_r_min;
    m_descWidth = Xi_descWidth;
    m_descHeight = Xi_descHeight;
  }

  /******************************************************************************
  *
  *: Method name: ~SLDRCR_RDI_CRegDictionary
  *
  ******************************************************************************/

  CRegDictionary::~CRegDictionary()
  {
    DeleteDescriptors();
  }


 


  /******************************************************************************
  *
  *: Method name: setParameters
  *
  ******************************************************************************/
  void CRegDictionary::setParameters(float Xi_r_max, float Xi_r_min, int Xi_descWidth, int Xi_descHeight)
  {
    m_r_max = Xi_r_max;
    m_r_min = Xi_r_min;
    m_descWidth = Xi_descWidth;
    m_descHeight = Xi_descHeight;
  }


  /******************************************************************************
  *
  *: Method name: getParameters
  *
  ******************************************************************************/
  void CRegDictionary::getParameters(float& Xo_r_max, float& Xo_r_min, int& Xo_descWidth, int& Xo_descHeight)
  {
    Xo_r_max = m_r_max;
    Xo_r_min = m_r_min;
    Xo_descWidth = m_descWidth;
    Xo_descHeight = m_descHeight;
  }



  /******************************************************************************
  *
  *: Method name: ResetDictionary
  *
  ******************************************************************************/
  void CRegDictionary::ResetDictionary()
  {
    DeleteDescriptors();

    ResetGrid();
  }


  /******************************************************************************
  *
  *: Method name: DeleteDescriptors
  *
  ******************************************************************************/
  void CRegDictionary::DeleteDescriptors()
  {
    if (m_descriptors != NULL)
    {
      for (int dicIndex = 0; dicIndex < m_size; dicIndex++)
      {
        delete[] m_descriptors[dicIndex];
        delete[] m_descriptorsDFT[dicIndex];

      }
      delete[] m_descriptors;
      delete[] m_descriptorsDFT;
      m_descriptors = NULL;
      m_descriptorsDFT = NULL;
    }
  }


  /******************************************************************************
  *
  *: Method name: DictionaryUpdate
  *
  ******************************************************************************/
  void CRegDictionary::DictionaryUpdate(int Xi_numPts, const CVec3* Xi_pts, float Xi_d_grid, float Xi_d_sensor)
  {
    PROFILE("DictionaryUpdate");

    int preSize = PointCloudAndGridUpdate(Xi_numPts, Xi_pts, Xi_d_grid, Xi_d_sensor);

    //create vectors for the descriptors:
    Features::resizeArray(m_descriptors, preSize, m_size);
    Features::resizeArray(m_descriptorsDFT, preSize, m_size);
  }




  /******************************************************************************
  *
  *: Method name: PCL2descriptor
  *
  ******************************************************************************/
  void CRegDictionary::PCL2descriptor(int Xi_numPts, const CVec3* Xi_pts, float* Xo_RangeImage)
  {
    //PROFILE("PCL2descriptor");
    int totalDescSize = m_descHeight * m_descWidth;
    float azimuthRes = m_descWidth / (2.0f * float(M_PI));
    float elevationRes = m_descHeight / float(M_PI);
    memset(Xo_RangeImage, 0, totalDescSize * sizeof(float));

    bool checkMin = !(m_r_min == -1);
    bool checkMax = !(m_r_max == -1);

    //put closest range per azimuth & elevation in the range image.
    for (int i = 0; i < Xi_numPts; i++)
    {
      float x = Xi_pts[i].x;
      float y = Xi_pts[i].y;
      float z = Xi_pts[i].z;
      float azimuth = atan2(y, x);
      float elevation = atan2(z, sqrt(x*x + y*y));
      float r = sqrt(x*x + y*y + z*z);

      if (checkMin && r < m_r_min)
        continue;
      if (checkMax && r > m_r_max)
        continue;

      int azimuthInds = int(floor((azimuth + M_PI) * azimuthRes));
      int elevationInds = m_descHeight - 1 - int(floor((elevation + M_PI / 2) * elevationRes));

      int index = elevationInds*m_descWidth + azimuthInds;

      if ((Xo_RangeImage[index] == 0) || (r < Xo_RangeImage[index]))
        Xo_RangeImage[index] = r;
    }
  }




  /******************************************************************************
  *
  *: Method name: Descriptor2DFT
  *
  ******************************************************************************/
  void CRegDictionary::Descriptor2DFT(float* Xi_Descriptor, std::complex<float>* Xo_DescriptorDFT)
  {
    //PROFILE("Descriptor2DFT");
    for (int index2 = 0; index2 < m_descHeight; index2++)
    {
      for (int index1 = 0; index1 < m_descWidth; index1++)
      {
        int index = (index2 * m_descWidth) + index1;
        Xo_DescriptorDFT[index] = Xi_Descriptor[index];
      }
    }

    IfrMath::DFT2D(unsigned int(m_descWidth), unsigned int(m_descHeight), Xo_DescriptorDFT);
  }




  /******************************************************************************
  *
  *: Method name: GetEntryDescriptorDFT
  *
  ******************************************************************************/
  std::complex<float>* CRegDictionary::GetEntryDescriptorDFT(int Xi_entryIndex)
  {
    if (m_descriptors[Xi_entryIndex] == NULL)
    {
      CVec3* ptsTran = new CVec3[m_numPtsMain];
      CMat4 orients = m_Orient[Xi_entryIndex];
      CVec3 Pos = CVec3(orients.m[3][0], orients.m[3][1], orients.m[3][2]);

      //transform main point cloud to grid point's orientation:
      #pragma omp parallel for
      for (int Index = 0; Index < m_numPtsMain; Index++)
      {
        //transform point:
        CVec3 PosShifted = m_ptsMain[Index] - Pos;
        CVec3 MatRow0 = CVec3(orients.m[0][0], orients.m[0][1], orients.m[0][2]);
        CVec3 MatRow1 = CVec3(orients.m[1][0], orients.m[1][1], orients.m[1][2]);
        CVec3 MatRow2 = CVec3(orients.m[2][0], orients.m[2][1], orients.m[2][2]);
        ptsTran[Index].x = DotProd(PosShifted, MatRow0);
        ptsTran[Index].y = DotProd(PosShifted, MatRow1);
        ptsTran[Index].z = DotProd(PosShifted, MatRow2);
      }


      //create descriptor - range image, DFT
      int totalDescSize = m_descHeight * m_descWidth;

      m_descriptors[Xi_entryIndex] = new float[totalDescSize];
      PCL2descriptor(m_numPtsMain, ptsTran, m_descriptors[Xi_entryIndex]);

      m_descriptorsDFT[Xi_entryIndex] = new std::complex<float>[totalDescSize];
      Descriptor2DFT(m_descriptors[Xi_entryIndex], m_descriptorsDFT[Xi_entryIndex]);

      delete[] ptsTran;


    }

    return m_descriptorsDFT[Xi_entryIndex];
  }




  /******************************************************************************
  *
  *: Method name: BestPhaseCorr
  *
  ******************************************************************************/
  void CRegDictionary::BestPhaseCorr(std::complex<float>* Xi_descriptorDFT0, std::complex<float>* Xi_descriptorDFT1, int& Xo_bestRow, int& Xo_bestCol, float& Xo_bestScore)
  {
    //PROFILE("BestPhaseCorr");
    Xo_bestScore = FLT_MIN;
    Xo_bestRow = 0;
    Xo_bestCol = 0;

    std::complex<float>* PhCor = new std::complex<float>[m_descWidth * m_descHeight];

    IfrMath::UnitPhaseCorrelation(Xi_descriptorDFT0, Xi_descriptorDFT1, PhCor, unsigned int(m_descWidth), unsigned int(m_descHeight));
    IfrMath::DFT2D(unsigned int(m_descWidth), unsigned int(m_descHeight), PhCor, false);
    IfrMath::DFTshift0ToOrigin(PhCor, unsigned int(m_descWidth), unsigned int(m_descHeight));

    //find max:
    for (int index2 = 0; index2 < m_descHeight; index2++)
    {
      for (int index1 = 0; index1 < m_descWidth; index1++)
      {
        int index = (index2 * m_descWidth) + index1;

        if (PhCor[index].real() > Xo_bestScore)
        {
          Xo_bestScore = PhCor[index].real();
          Xo_bestRow = index2;
          Xo_bestCol = index1;
        }
      }
    }

    delete[] PhCor;
  }




  /******************************************************************************
  *
  *: Method name: SearchDictionary
  *
  ******************************************************************************/
  int CRegDictionary::SearchDictionary(int Xi_maxCandidates, float Xi_searchRadius, std::complex<float>* Xi_descriptorDFT, int* Xo_candidates, float* Xo_grades, CMat4* Xo_orientations, const CVec3& Xi_estimatePos)
  {
    PROFILE("SearchDictionary");
    float finalMax = FLT_MIN;
    int finalIndex = -1;
    int finalrow = -1;
    int finalcol = -1;


    int NumOfCandidates = 0;
    int minIndex = 0;
    Xo_grades[minIndex] = FLT_MAX;

    int* bestCols = new int[Xi_maxCandidates];

    std::vector<int> inSearchRadius;


    for (int gridIndex = 0; gridIndex < m_size; gridIndex++)
    {
      CMat4 orients = m_Orient[gridIndex];
      CVec3 Pos = CVec3(orients.m[3][0], orients.m[3][1], orients.m[3][2]);

      //ignore entries which are too far from the estimation:
      if (Dist(Xi_estimatePos, Pos) <= Xi_searchRadius)
        inSearchRadius.push_back(gridIndex);
    }

    #pragma omp parallel for
    for (int inSrIndex = 0; inSrIndex < inSearchRadius.size(); inSrIndex++)
      GetEntryDescriptorDFT(inSearchRadius[inSrIndex]);


    int inSrIndex = 0;
    //take first Xi_maxCandidates entries from dictionary which were considered close enough to estimation:
    for (inSrIndex; inSrIndex < inSearchRadius.size(); inSrIndex++)
    {
      int gridIndex = inSearchRadius[inSrIndex];

      std::complex<float>* gridDescDFT = GetEntryDescriptorDFT(gridIndex);
      float bestMax = FLT_MIN;
      int bestRow = -1;
      int bestCol = -1;
      BestPhaseCorr(gridDescDFT, Xi_descriptorDFT, bestRow, bestCol, bestMax);

      Xo_candidates[NumOfCandidates] = gridIndex;
      Xo_grades[NumOfCandidates] = bestMax;
      bestCols[NumOfCandidates] = bestCol;

      if (bestMax < Xo_grades[minIndex])
      {
        minIndex = NumOfCandidates;
      }

      NumOfCandidates++;

      if (NumOfCandidates == Xi_maxCandidates)
        break;
    }


    //continue combing dictionary (remaining with best Xi_maxCandidates Candidates:
    for (inSrIndex; inSrIndex < inSearchRadius.size(); inSrIndex++)
    {
      int gridIndex = inSearchRadius[inSrIndex];

      std::complex<float>* gridDescDFT = GetEntryDescriptorDFT(gridIndex);
      float bestMax = FLT_MIN;
      int bestRow = -1;
      int bestCol = -1;
      BestPhaseCorr(gridDescDFT, Xi_descriptorDFT, bestRow, bestCol, bestMax);
      if (bestMax > Xo_grades[minIndex])
      {
        Xo_grades[minIndex] = bestMax;
        Xo_candidates[minIndex] = gridIndex;
        bestCols[minIndex] = bestCol;


        for (int candIndex = 0; candIndex < NumOfCandidates; candIndex++)
        {
          if (Xo_grades[candIndex] < Xo_grades[minIndex])
            minIndex = candIndex;
        }
      }
    }


    //compute rotation matrix for candidates:
    float azimuthRes = 2 * float(M_PI) / m_descWidth;
    float centerShift = ((m_descWidth & 1) == 0) ? 0.5f : 0.0f;

    #pragma omp parallel for
    for (int candIndex = 0; candIndex < NumOfCandidates; candIndex++)
    {
      CMat4 orients = m_Orient[Xo_candidates[candIndex]];
      CVec3 Pos = CVec3(orients.m[3][0], orients.m[3][1], orients.m[3][2]);

      //calc beast azimuth in radians.
      float peak_azimuth = float(bestCols[candIndex] + centerShift) * azimuthRes - float(M_PI);

      //create corresponding rotation matrix and calc final orientation:
      float OrientVals[] = { cos(peak_azimuth), -sin(peak_azimuth), 0.0f, 0.0f,
        sin(peak_azimuth),  cos(peak_azimuth), 0.0f, 0.0f,
        0.0f             ,  0.0f             , 1.0f, 0.0f,
        0.0f             ,  0.0f             , 0.0f, 1.0f };


      TransposeLeftMultiply(orients, CMat4(OrientVals), Xo_orientations[candIndex]);
      Xo_orientations[candIndex].m[3][0] = Pos.x;
      Xo_orientations[candIndex].m[3][1] = Pos.y;
      Xo_orientations[candIndex].m[3][2] = Pos.z;
    }

    delete[] bestCols;

    #ifdef DEBUG_LOCAL_RANGE_IMAGE //DEBUG - SearchDictionary print candidates

    int bestIndex = 0;
    float bestGrade = Xo_grades[0];
    for (int index = 1; index < NumOfCandidates; index++)
    {
      if (Xo_grades[index] > bestGrade)
      {
        bestIndex = index;
        bestGrade = Xo_grades[index];
      }
    }
    for (int index = 0; index < NumOfCandidates; index++)
    {
      CRegDebug debugDic("L:\\code\\SLDR\\sldrcr\\Debug");

      char fileName[100] = "Candidate_";
      char indexC[10];  itoa(index, indexC, 10);
      char candC[10];   itoa(Xo_candidates[index], candC, 10);
      char gradeC[10];  itoa(int(Xo_grades[index] * 100), gradeC, 10);

      if (index == bestIndex)
      {
        strcat(fileName, indexC); strcat(fileName, "_best_");
      }
      else
      {
        strcat(fileName, indexC); strcat(fileName, "_");
      }
      strcat(fileName, candC);  strcat(fileName, "_");
      strcat(fileName, gradeC); strcat(fileName, ".bmp");

      debugDic.SaveAsBmp(fileName, m_descriptors[Xo_candidates[index]], m_descWidth, m_descHeight, 2, 60);//Xi_r_min, Xi_r_max); //SearchDictionary debug
    }
    #endif


    return NumOfCandidates;
  }


  /******************************************************************************
  *                             Protected methods                               *
  ******************************************************************************/
  



  /******************************************************************************
  *                              Private methods                                *
  ******************************************************************************/


  /******************************************************************************
  *                            EXPORTED FUNCTIONS                               *
  ******************************************************************************/

  /******************************************************************************
  *                            INTERNAL FUNCTIONS                               *
  ******************************************************************************/







} //namespace SLDR