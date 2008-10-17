% I got this code from Matlab file exchange
% http://www.mathworks.com/matlabcentral/fileexchange/loadFile.do?objectId=7933&objectType=file
% Author: John McArthur (University of Southern California)
% his email: johnnyfisma@hotmail.com
% http://www.mathworks.com/matlabcentral/fileexchange/loadAuthor.do?objectType=author&objectId=1094612

function Output = readFileInCells(file)

%Location of Data File
root=file;

%Open File and count the number of rows in the file
fid=fopen(root);
nRows=0;
while 1
    iString=fgetl(fid);
    if ~ischar(iString)
        break
    end
    nRows=nRows+1;
end

%Return to beginning of file
fseek(fid,0,'bof');

%For each row, assign each space delimitted object to a cell in the "Output" matrix
for iRow=1:nRows
    iCol=1;
    %Temporary storage of the first object
    %   Note: the space delimitter used here can be replaced by any delimitter
    [TempOutput,Rem]=strtok(fgetl(fid),' ');
    %If there is now data on this row, then assign the first object to be an underscore
    if (length(TempOutput) == 0)
        TempOutput='_';
    end
    %Build the "Output" matrix this will be the first column of the iRow-th row
    Output(iRow,iCol)=cellstr(TempOutput);
    %Repeat this only using Rem as the total string and incrementing the iCol counter
    while length(Rem) > 0
        iCol=iCol+1;
        [TempOutput,Rem]=strtok(Rem,' ');            
        Output(iRow,iCol)=cellstr(TempOutput);
    end
end
