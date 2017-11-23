/*
	Still working on the UI for this, was going to have a UI where you can enter a equation,
	but while I work on that I made the graphing part in this seperate project.
*/
// include the basic windows header file
#include "stdafx.h"
#include <windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <vector>

// include the Direct3D Library file
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dx11.lib")
#pragma comment (lib, "d3dx10.lib")


// define the screen resolution
#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600


// a struct to define a single vertex
struct VERTEX { FLOAT X, Y, Z; D3DXCOLOR Color; };
struct VARIABLE { float coef; float exp; };

// global declarations
IDXGISwapChain *swapchain;             // the pointer to the swap chain interface
ID3D11Device *dev;                     // the pointer to our Direct3D device interface
ID3D11DeviceContext *devcon;           // the pointer to our Direct3D device context
ID3D11RenderTargetView *backbuffer;    // the pointer to our back buffer
ID3D11InputLayout *pLayout;            // the pointer to the input layout
ID3D11VertexShader *pVS;               // the pointer to the vertex shader
ID3D11PixelShader *pPS;                // the pointer to the pixel shader
ID3D11Buffer *pVBuffer;                // the pointer to the vertex buffer
ID3D11Buffer *pVBuffer2;
int axisVertexSize = 0;


// function prototypes
void InitD3D(HWND hWnd);    // sets up and initializes Direct3D
void RenderFrame(void);     // renders a single frame
void CleanD3D(void);        // closes Direct3D and releases memory
void InitGraphics(const std::vector<VARIABLE> &equation, float xMin, float xMax, float yMin, float yMax);    // creates the shape to render
void InitPipeline(void);    // loads and prepares the shaders

							// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam);

// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	// the handle for the window, filled by a function
	HWND hWnd;
	// this struct holds information for the window class
	WNDCLASSEX wc;

	// clear out the window class for use
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	// fill in the struct with the needed information
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	// wc.hbrBackground = (HBRUSH)COLOR_WINDOW;    // no longer needed
	wc.lpszClassName = L"WindowClass1";

	// register the window class
	RegisterClassEx(&wc);

	RECT wr = { 0, 0, 500, 400 };    // set the size, but not the position
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);    // adjust the size

														  
	// create the window and use the result as the handle
	hWnd = CreateWindowEx(NULL,
		L"WindowClass1",
		L"Our First Windowed Program",
		WS_OVERLAPPEDWINDOW,
		300,    // x-position of the window
		300,    // y-position of the window
		wr.right - wr.left,    // width of the window
		wr.bottom - wr.top,    // height of the window
		NULL,
		NULL,
		hInstance,
		NULL);

				  // display the window on the screen
	ShowWindow(hWnd, nCmdShow);

	// set up and initialize Direct3D
	InitD3D(hWnd);

	// enter the main loop:

	// this struct holds Windows event messages
	MSG msg = { 0 };

	// Enter the infinite message loop
	while (TRUE)
	{
		// Check to see if any messages are waiting in the queue
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// translate keystroke messages into the right format
			TranslateMessage(&msg);

			// send the message to the WindowProc function
			DispatchMessage(&msg);

			// check to see if it's time to quit
			if (msg.message == WM_QUIT)
				break;
		}
		else
		{
			// Run game code here
			// ...
			// ...
		}

		RenderFrame();
	}

	// clean up DirectX and COM
	CleanD3D();

	// return this part of the WM_QUIT message to Windows
	return msg.wParam;
}

// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// sort through and find what code to run for the message given
	switch (message)
	{
		case WM_DESTROY:
			// close the application entirely
			PostQuitMessage(0);
			return 0;
			break;
	}

	// Handle any messages the switch statement didn't
	return DefWindowProc(hWnd, message, wParam, lParam);
}

// this function initializes and prepares Direct3D for use
void InitD3D(HWND hWnd)
{
	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	// clear out the struct for use
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferCount = 1;                                    // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	scd.BufferDesc.Width = SCREEN_WIDTH;                    // set the back buffer width
	scd.BufferDesc.Height = SCREEN_HEIGHT;                  // set the back buffer height
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	scd.OutputWindow = hWnd;                                // the window to be used
	scd.SampleDesc.Count = 4;                               // how many multisamples
	scd.Windowed = TRUE;                                    // windowed/full-screen mode
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;     // allow full-screen switching

															// create a device, device context and swap chain using the information in the scd struct
	D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&swapchain,
		&dev,
		NULL,
		&devcon);

	// get the address of the back buffer
	ID3D11Texture2D *pBackBuffer;
	swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	// use the back buffer address to create the render target
	dev->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer);
	pBackBuffer->Release();

	// set the render target as the back buffer
	devcon->OMSetRenderTargets(1, &backbuffer, NULL);

	// Set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = SCREEN_WIDTH;
	viewport.Height = SCREEN_HEIGHT;

	devcon->RSSetViewports(1, &viewport);

	InitPipeline();
	////////////////////////////////////////////////////////////////////////////////////////////////////////// Here's where you can edit the function ////////////////////////////////////////////////////////////
	std::vector<VARIABLE> equation = { {1.0f, -2.0f}, {3.0f, 2.0f} }; // ......   Struct is coefficient, exponent  {2.0f,3.0f} = 2x^3,  { {2.0f,3.0f,} , {3.0f,1.0f} } = 2x^3 + 3x
	//Problem: if there is a hole where both sides are going opposite directions, the linestrip will draw a line across the whole graph, like 1/x

	InitGraphics(equation, -10, 10, -10, 10);          //////////// You can change the graph size here ///////////////////////////////////////////////////////////////////////////////////////////////////////////
}

// this is the function used to render a single frame
void RenderFrame(void)
{
	// clear the back buffer to a deep blue
	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	devcon->ClearRenderTargetView(backbuffer, color);

	// do 3D rendering on the back buffer here
	// select which vertex buffer to display
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	devcon->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);

	// select which primtive type we are using
	devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	
	devcon->Draw(axisVertexSize, 0);

	// Second Buffer ////////////////// 
	devcon->IASetVertexBuffers(0, 1, &pVBuffer2, &stride, &offset);

	devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	//draw the function
	devcon->Draw(SCREEN_WIDTH, 0);

	// switch the back buffer and the front buffer
	swapchain->Present(0, 0);
}


// this is the function that cleans up Direct3D and COM
void CleanD3D(void)
{
	swapchain->SetFullscreenState(FALSE, NULL);    // switch to windowed mode

												   // close and release all existing COM objects
	pLayout->Release();
	pVS->Release();
	pPS->Release();
	pVBuffer->Release();
	pVBuffer2->Release();
	swapchain->Release();
	backbuffer->Release();
	dev->Release();
	devcon->Release();
}

// this is the function that creates the shape to render
void InitGraphics(const std::vector<VARIABLE> &equation, float xMin, float xMax, float yMin, float yMax)
{
	if (xMax <= xMin) xMin = xMax - 1; //does something so the min isnt bigger than or equal to the max
	if (yMax <= yMin) yMin = yMax - 1;
	//Ranges
	float xRange = xMax - xMin;
	float yRange = yMax - yMin;
	//Conversion constants
	const float X_TO_1 = 2 / xRange;
	const float Y_TO_1 = 2 / yRange;

	// create a triangle using the VERTEX struct
	const D3DXCOLOR BLACK = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	VERTEX OurVertices[4];

	//Setting up the axis'

	if (yMin <= 0 && yMax >= 0)
	{
		//draw x axis
		axisVertexSize = 2;
		OurVertices[0] = { -1.0f, (-yMin) * Y_TO_1 - 1.0f, 0.0f, BLACK };
		OurVertices[1] = { 1.0f, (-yMin) * Y_TO_1 - 1.0f, 0.0f, BLACK };
	}
	if (xMin <= 0 && xMax >= 0)
	{
		//draw x axis
		axisVertexSize += 2;
		OurVertices[axisVertexSize - 2] = { (-xMin) * X_TO_1 - 1.0f, -1.0f, 0.0f, BLACK };
		OurVertices[axisVertexSize - 1] = { (-xMin) * X_TO_1 - 1.0f, 1.0f, 0.0f, BLACK };
	}

	// create the vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
	bd.ByteWidth = sizeof(OurVertices);             // size is the VERTEX struct * 3
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer

	dev->CreateBuffer(&bd, NULL, &pVBuffer);       // create the buffer


												   // copy the vertices into the buffer
	D3D11_MAPPED_SUBRESOURCE ms;
	devcon->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
	memcpy(ms.pData, OurVertices, sizeof(OurVertices));                 // copy the data
	devcon->Unmap(pVBuffer, NULL);                                      // unmap the buffer

	////////////////  Second Buffer //////////////////////////////////////////

	const float INCREMENT = xRange / float(SCREEN_WIDTH);
	const D3DXCOLOR RED = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f);
	VERTEX functionVertices[SCREEN_WIDTH];

	int i = 0;
	for (float x = xMin; i < SCREEN_WIDTH; x += INCREMENT, i++)
	{
		float y = 0.0f;  //resets the y to 0 for each x
		for (VARIABLE var : equation)
		{
			y += var.coef * pow(x, var.exp);   //goes through all the polynomials to calculate the y variable
		}

		if (y > yMax) y = yMax;
		else if (y < yMin) y = yMin;
		//Shifts the range to start at 0, then scales it down to a range of 2, then minuses one.
		functionVertices[i] = { (x - xMin) * X_TO_1 - 1.0f, (y - yMin) * Y_TO_1 - 1.0f, 0.0f, RED }; // MINUS ONE IS (RANGE/2)/(RANGE/2), ITS ONLY BECAUSE WE ARE ALWAYS CONVERTING TO -1 TO 1
	}


	// create the vertex buffer
	D3D11_BUFFER_DESC bd2;
	ZeroMemory(&bd2, sizeof(bd2));

	bd2.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
	bd2.ByteWidth = sizeof(functionVertices);             // size is the VERTEX struct * 3
	bd2.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
	bd2.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer

	dev->CreateBuffer(&bd2, NULL, &pVBuffer2);

	D3D11_MAPPED_SUBRESOURCE ms2;
	devcon->Map(pVBuffer2, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms2);
	memcpy(ms2.pData, functionVertices, sizeof(functionVertices));
	devcon->Unmap(pVBuffer2, NULL);
}


// this function loads and prepares the shaders
void InitPipeline()
{
	// load and compile the two shaders
	ID3D10Blob *VS, *PS;
	D3DX11CompileFromFile(L"shaders.shader", 0, 0, "VShader", "vs_4_0", 0, 0, 0, &VS, 0, 0);
	D3DX11CompileFromFile(L"shaders.shader", 0, 0, "PShader", "ps_4_0", 0, 0, 0, &PS, 0, 0);

	// encapsulate both shaders into shader objects
	dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &pVS);
	dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &pPS);

	// set the shader objects
	devcon->VSSetShader(pVS, 0, 0);
	devcon->PSSetShader(pPS, 0, 0);

	// create the input layout object
	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	dev->CreateInputLayout(ied, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &pLayout);
	devcon->IASetInputLayout(pLayout);
}

