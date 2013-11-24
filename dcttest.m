#!/usr/bin/octave

function dcttest
clear;

A = 	[136, 142, 148, 161;
	 138, 145, 149, 164;
	 131, 143, 150, 158;
	 135, 139, 149, 162;];

disp ("Input matrix 4x4:"), disp (A);

B = dct2(A);

disp ("Output matrix after DCT:"), disp (B);

C = idct2(B);

disp ("Output matrix after IDCT:"), disp (C);


A = 	[ 123, 123, 100, 190, 100, 50, 255, 240;
	  122, 156, 155, 145, 121, 70, 230, 220;
	  122, 156, 155, 145, 121, 70, 230, 220;
	  122, 156, 155, 145, 121, 70, 230, 220;
	  122, 156, 155, 145, 121, 70, 230, 220;
	  122, 156, 155, 145, 121, 70, 230, 220;
	  122, 156, 155, 145, 121, 70, 230, 220;
	  122, 156, 155, 145, 121, 70, 230, 220;];

disp ("Input matrix 8x8:"), disp (A);

B = dct2(A);

disp ("Output matrix after DCT:"), disp (B);

C = idct2(B);

disp ("Output matrix after IDCT:"), disp (C);

endfunction

dcttest
