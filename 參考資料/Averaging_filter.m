%% Averaging Filter
%
% This example shows the recommended workflow for generating C code from  a
% MATLAB function using the 'codegen' command.
% These are the steps:  
% 1. Add the %#codegen directive to the MATLAB function to indicate that it
% is intended for code generation. This directive also enables  the MATLAB 
% code analyzer to identify warnings and errors specific to MATLAB for 
% code generation.
%
% 2. Generate a MEX function to check that the MATLAB code is suitable for
% code generation. If errors occur, you should fix them before generating
% C code.
%
% 3. Test the MEX function in MATLAB to ensure that it is functionally 
% equivalent to the original MATLAB code and that no run-time errors occur.
%
% 4. Generate C code.
%
% 5. Inspect the C code. 
%
%   Copyright 2010-2011 The MathWorks, Inc.
%% Prerequisites
% There are no prerequisites for this example.
%
%% Create a New Folder and Copy Relevant Files
% The following code will create a folder in your current
% working folder (pwd). The new folder will only contain the files
% that are relevant for this example. If you do not want to affect the
% current folder (or if you cannot generate files in this folder),
% you should change your working folder.
%% Run Command: Create a New Folder and Copy Relevant Files
coderdemo_setup('coderdemo_averaging_filter');

%% About the 'averaging_filter' Function
% The <matlab:edit(fullfile(matlabroot,'toolbox','coder','codegendemos','coderdemo_averaging_filter.m')) averaging_filter.m>
% function acts as an averaging filter on the input signal; it takes an 
% input vector of values and computes an average for each value in the 
% vector. The output vector is the same size and shape as the input vector.
type averaging_filter
%%
% The %#codegen compilation directive indicates that the MATLAB code is
% intended for code generation.
%% Create some Sample Data
% Generate a noisy sine wave and plot the result.
v = 0:0.00614:2*pi;
x = sin(v) + 0.3*rand(1,numel(v));
plot(x, 'red');
 
%% Generate a MEX Function for Testing
% Generate a MEX function using the 'codegen' command. The 'codegen'
% command checks that the MATLAB function is suitable for code generation
% and generates a MEX function that you can test in MATLAB prior to 
% generating C code.
%
codegen averaging_filter -args {x}
%%
% Because C uses static typing, 'codegen' must determine the properties of 
% all variables in the MATLAB files at compile time. Here, the '-args'
% command-line option supplies an example input so that 'codegen' can infer
% new types based on the input types.  Using the sample signal created 
% above as the example input ensures that the MEX function can use
% the same input.
%
% By default, 'codegen' generates a MEX function named 'averaging_filter_mex'
% in the current folder. This allows you to test the MATLAB code and MEX 
% function and compare the results.
%% Test the MEX Function in MATLAB
% Run the MEX function in MATLAB
y = averaging_filter_mex(x);
% Plot the result when the MEX function is applied to the noisy sine wave.
% The 'hold on' command ensures that the plot uses the same figure window as
% the previous plot command.
hold on;
plot(y, 'blue');

%% Generate C Code 
codegen -config coder.config('lib') averaging_filter -args {x}

%% Inspect the Generated Code
% The 'codegen' command with the '-config coder.config('lib')' option 
% generates C code packaged as a standalone C library.
% The generated C code is in the 'codegen/lib/averaging_filter/' folder.
% The files are:
dir codegen/lib/averaging_filter/

%% Inspect the C Code for the 'averaging_filter.c' Function
type codegen/lib/averaging_filter/averaging_filter.c

%% Cleanup
% Remove files and return to original folder
%% Run Command: Cleanup
cleanup

displayEndOfDemoMessage(mfilename)
