// Modified from a modified version of the file described in the following comment...
//    -Chris

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: Medical1.cxx,v $
  Language:  C++
  Date:      $Date: 2002/11/27 16:06:38 $
  Version:   $Revision: 1.2 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/


#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkVolume16Reader.h"
#include "vtkPolyDataMapper.h"
#include "vtkDataSetMapper.h"
#include "vtkFixedPointVolumeRayCastMapper.h"
#include "vtkActor.h"
#include "vtkOutlineFilter.h"
#include "vtkCamera.h"
#include "vtkProperty.h"
#include "vtkPolyDataNormals.h"
#include "vtkContourFilter.h"
#include "vtkStructuredPointsReader.h"
#include "vtkMarchingCubes.h"
#include "vtkRecursiveDividingCubes.h"
#include "vtkScalarBarWidget.h"
#include "vtkScalarBarActor.h"

class KeyInterpreter : public vtkCommand {
public:
    // Allocator
    static KeyInterpreter* New(void) { return new KeyInterpreter; }

    // Internal data.  This needs to be set immediately after allocation.
    vtkFixedPointVolumeRayCastMapper* map;
    vtkContourFilter* contourExtractor1;
    vtkContourFilter* contourExtractor2;
    vtkRenderer* renderer;
    vtkActor* outline;
    vtkActor* contour1;
    vtkActor* contour2;
    vtkVolume* volume;
    vtkColorTransferFunction* colorTransferFunction;
    vtkPiecewiseFunction* opacityTransferFunction;
    vtkColorTransferFunction* volumeColorTransferFunction;
    vtkPiecewiseFunction* volumeOpacityTransferFunction;

    // Variables for iso-value and ray step size
    double stepSize;
    double isoValue1 = 100;
    double isoValue2 = 100;

    KeyInterpreter() {
        stepSize = 1.0; // Initial step size
    }

    int mode = 0;

    // Called upon the registered event (i.e., a key press)
    void Execute(vtkObject* caller, unsigned long eventId, void* callData) {
        double dist;
        //int point;
        vtkRenderWindowInteractor* iren =
            reinterpret_cast<vtkRenderWindowInteractor*>(caller);
        switch (iren->GetKeyCode()) {
        case '=':

            isoValue1 += 10.0; // Increase iso-value for contourExtractor1
            isoValue2 += 10.0; // Increase iso-value for contourExtractor2
            contourExtractor1->SetValue(0, isoValue1);
            contourExtractor2->SetValue(0, isoValue2);
            cout << "The isovalue1 now is " << isoValue1 << endl;
            cout << "The isovalue2 now is " << isoValue2 << endl;


            break;
        case '-':

            isoValue1 -= 10.0; // Decrease iso-value for contourExtractor1
            isoValue2 -= 10.0; // Decrease iso-value for contourExtractor2
            if (isoValue1 < 0.0) isoValue1 = 0.0; // Ensure it doesn't go below 0
            if (isoValue2 < 0.0) isoValue2 = 0.0; // Ensure it doesn't go below 0
            contourExtractor1->SetValue(0, isoValue1);
            contourExtractor2->SetValue(0, isoValue2);
            cout << "The isovalue1 now is " << isoValue1 << endl;
            cout << "The isovalue2 now is " << isoValue2 << endl;

            break;
            iren->Render();
        }
    }

};


int main(int argc, char** argv)
{

    // Create the renderer, the render window, and the interactor. The renderer
    // draws into the render window, the interactor enables mouse- and 
    // keyboard-based interaction with the data within the render window.
    vtkRenderer* aRenderer = vtkRenderer::New();
    vtkRenderWindow* renWin = vtkRenderWindow::New();
    renWin->AddRenderer(aRenderer);
    vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::New();
    iren->SetRenderWindow(renWin);


    // vtkVolumeReader16 reads in the head CT data set.  
    vtkVolume16Reader* reader = vtkVolume16Reader::New();
    reader->SetDataDimensions(256, 256);
    //reader->SetImageRange(1, 178);
    reader->SetImageRange(1, 44);
    reader->SetDataByteOrderToLittleEndian();
    reader->SetFilePrefix("../data/frog/frog2ci");
    //reader->SetDataSpacing(1, 1, 1);
    reader->SetDataSpacing(1, 1, 2.2);


    // This next section creates two contours for the density data.  A
    //    vtkContourFilter object is created that takes the input data from
    //    the reader.																		
    vtkContourFilter* contourExtractor = vtkContourFilter::New();
    contourExtractor->SetInputConnection(reader->GetOutputPort());
    contourExtractor->SetValue(0, 100);

    vtkContourFilter* contourExtractor2 = vtkContourFilter::New();
    contourExtractor2->SetInputConnection(reader->GetOutputPort());
    contourExtractor2->SetValue(0, 100);


    // This section creates the polygon normals for the contour surfaces
    //    and creates the mapper that takes in the newly normalized surfaces
    vtkPolyDataNormals* contourNormals = vtkPolyDataNormals::New();
    contourNormals->SetInputConnection(contourExtractor->GetOutputPort());
    contourNormals->SetFeatureAngle(60.0);
    vtkPolyDataMapper* contourMapper = vtkPolyDataMapper::New();
    contourMapper->SetInputConnection(contourNormals->GetOutputPort());
    contourMapper->ScalarVisibilityOff();
    vtkPolyDataNormals* contourNormals2 = vtkPolyDataNormals::New();
    contourNormals2->SetInputConnection(contourExtractor2->GetOutputPort());
    contourNormals2->SetFeatureAngle(60.0);
    vtkPolyDataMapper* contourMapper2 = vtkPolyDataMapper::New();
    contourMapper2->SetInputConnection(contourNormals2->GetOutputPort());
    contourMapper2->ScalarVisibilityOff();


    // This section sets up the Actor that takes the contour
    //    This is where you can set the color and opacity of the two contours
    vtkActor* contour = vtkActor::New();
    contour->SetMapper(contourMapper);
    contour->GetProperty()->SetColor(0.8, 0.4, 0.0);
    contour->GetProperty()->SetOpacity(0.3);
    vtkActor* contour2 = vtkActor::New();
    contour2->SetMapper(contourMapper2);
    contour2->GetProperty()->SetColor(0.8, 0.8, 0.8);
    contour2->GetProperty()->SetOpacity(1.0);

    KeyInterpreter* key = KeyInterpreter::New();
    key->renderer = aRenderer;
    key->contourExtractor1 = contourExtractor;
    key->contourExtractor2 = contourExtractor2;
    key->contour1 = contour;
    key->contour2 = contour2;
    iren->AddObserver(vtkCommand::KeyPressEvent, key);


    // An outline provides context around the data.
    vtkOutlineFilter* outlineData = vtkOutlineFilter::New();
    outlineData->SetInputConnection(reader->GetOutputPort());
    vtkPolyDataMapper* mapOutline = vtkPolyDataMapper::New();
    mapOutline->SetInputConnection(outlineData->GetOutputPort());
    vtkActor* outline = vtkActor::New();
    outline->SetMapper(mapOutline);
    outline->GetProperty()->SetColor(0, 0, 0);

    // It is convenient to create an initial view of the data. The FocalPoint
    // and Position form a vector direction. Later on (ResetCamera() method)
    // this vector is used to position the camera to look at the data in
    // this direction.
    vtkCamera* aCamera = vtkCamera::New();
    aCamera->SetViewUp(0, 0, -1);
    aCamera->SetPosition(0, 1, 0);
    aCamera->SetFocalPoint(0, 0, 0);
    aCamera->ComputeViewPlaneNormal();

    // Actors are added to the renderer. An initial camera view is created.
    // The Dolly() method moves the camera towards the FocalPoint,
    // thereby enlarging the image.
    aRenderer->AddActor(outline);
    aRenderer->AddActor(contour);
    aRenderer->AddActor(contour2);
    aRenderer->SetActiveCamera(aCamera);
    aRenderer->ResetCamera();
    aCamera->Dolly(1.5);

    // Set a background color for the renderer and set the size of the
    // render window (expressed in pixels).
    aRenderer->SetBackground(1, 1, 1);
    renWin->SetSize(800, 600);

    // Note that when camera movement occurs (as it does in the Dolly()
    // method), the clipping planes often need adjusting. Clipping planes
    // consist of two planes: near and far along the view direction. The 
    // near plane clips out objects in front of the plane; the far plane
    // clips out objects behind the plane. This way only what is drawn
    // between the planes is actually rendered.
    aRenderer->ResetCameraClippingRange();


    // Initialize the event loop and then start it.
    iren->Initialize();
    renWin->SetWindowName("Simple Volume Renderer");
    renWin->Render();
    iren->Start();

    // It is important to delete all objects created previously to prevent
    // memory leaks. In this case, since the program is on its way to
    // exiting, it is not so important. But in applications it is
    // essential.
    reader->Delete();
    contourExtractor->Delete();
    contourNormals->Delete();
    contourMapper->Delete();
    contour->Delete();
    outlineData->Delete();
    mapOutline->Delete();
    outline->Delete();
    aCamera->Delete();
    iren->Delete();
    renWin->Delete();
    aRenderer->Delete();

    return 0;
}