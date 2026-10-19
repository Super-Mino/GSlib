#ifndef _SDL3_GSlib_IMGUI_HPP_
#define _SDL3_GSlib_IMGUI_HPP_

#include <SDL3/SDL.h>
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_sdl3.h"
#include "ImGui/imgui_impl_sdlrenderer3.h"

namespace gs {

bool InitImGui(SDL_Window* win=nullptr, SDL_Renderer* ren=nullptr);
void ImGuiNewFrame();
void RenderImGuiFrame(SDL_Renderer* ren);void RenderTheImGuiFrame(SDL_Renderer* ren);void ImGuiRenderFrame(SDL_Renderer* ren);
void QuitImGui();


#endif //_SDL3_GSlib_IMGUI_HPP_


} //namespace gs





namespace gs 
{


bool InitImGui(SDL_Window* win, SDL_Renderer* ren)
{	
	if( win == nullptr )
		return false;
		
	if( ren == nullptr )
		return false;
			
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	
	ImGui_ImplSDL3_InitForSDLRenderer(win, ren);
	ImGui_ImplSDLRenderer3_Init(ren);	
	
	return true;
}


void ImGuiNewFrame()
{
	ImGui_ImplSDLRenderer3_NewFrame();	
	ImGui_ImplSDL3_NewFrame();	
	ImGui::NewFrame();
}


void RenderImGuiFrame(SDL_Renderer* ren)
{
	ImGui::Render();
	ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), ren);
}

void RenderTheImGuiFrame(SDL_Renderer* ren)
{
	return RenderImGuiFrame(ren);
}

void ImGuiRenderFrame(SDL_Renderer* ren)
{
	return RenderImGuiFrame(ren);
}


void QuitImGui()
{
	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
}


} //namespace gs



