classdef xbee < handle
    % XBEE Construct Xbee module object
    %
    %   x = xbee('SerialPortNum') constructs an xbee object connected to
    %   the xbee explorer using serial/USB port number
    %   (SerialPortNum).
    
    %   Copyright 2013-2014 The MathWorks, Inc.
    %   Author: Eun Kyung Lee
    
    % Define visible properties
    properties (SetAccess = private)
        % Port is assigned during connection and cannot be changed after.
        Port;
    end
    
    properties(Dependent = true)
        % BaudRate is assigned during connection, but can be changed after.
        BaudRate;
    end
    
    % Define constant properties
    properties (Constant = true)
        ConfigurableDigitalPins = [4 6 7 11 12 15 16 17 18 19 20];
        ConfigurableAnalogPins  = [17 18 19 20];
    end
    
    % Define public methods
    methods
        
        function [output] = discoverNetwork(obj)           
            % DISCOVERNETWORK Identifies the available XBee nodes in the
            % network by calling the 'ND' command.
            % 
            % Only the 64-bit address of the XBee nodes is returned by
            % this function. The address of the coordinator will not be 
            % included in the output.
            
            % Send 'ND' command.
            responses = obj.atCommand('ND');
            
            % Parse the responses to find the addresses of only the
            % non-coordinator nodes.
            validNodeCount = 1;
            for r = 1:numel(responses);
               
                % Get the address (bytes 3-10)
                if numel(responses{r})>=10
                    temp = dec2hex(responses{r}(3:10),2)';
                    routeraddr = char(temp(:)');
                
                    % Remove the 'NI' response string.
                    if numel(responses{r})>=12
                        nilength = find(responses{r}(11:end)==0,1);
                        if(isempty(nilength))
                            afterni = [];
                        else
                            afterni = responses{r}(11+nilength:end);
                        end
                    else
                        afterni = [];
                    end

                    % Get the parent node and device type.
                    if numel(afterni)>=3
                        parent = afterni(1)*256+afterni(2);
                        devtype = afterni(3);

                        % Keep node only if it is not the coordinator.
                        if(parent ~= 65534 || devtype ~= 0)
                            discoveredNodes{validNodeCount} = routeraddr;
                            validNodeCount = validNodeCount+1;
                        end
                    else
                        % If anything goes wrong, keep node to be safe.
                        discoveredNodes{validNodeCount} = routeraddr;
                        validNodeCount = validNodeCount+1;
                    end
                end
            end
            
            if nargout == 0
                disp(discoveredNodes');
            elseif nargout == 1
                output =  discoveredNodes';
            end
        end
        
        function setPANID(obj, varargin)
            %SETPANID Set the network parameters of XBee object.
            %   SETPANID sets the network parameters of an XBee module. This version
            %   only support changing PANID ('ID') and Channel Verification ('JV') tag.
            %
            %   SETPANID(x) automatically detects whether the USB/Serial connected
            %   XBee module is a Coordinator or Router, and sets the default PAN ID
            %   (6975) and Channel Verification tag.
            %
            %   SETPANID(x, 1111) automatically detects whether the USB/Serial
            %   connected XBee module is a Coordinator or Router, and sets the PAN ID
            %   to 1111 and the Channel Verification tag. Note that 1111 is a decimal
            %   number.
            
            minargs = 1;
            maxargs = 2;
            narginchk(minargs, maxargs);
            
            if nargin == 2
                if varargin{1} < 0 ||  varargin{1} > 65535
                    error('xbee:setPANID:PanIDChk', 'Range of your PAN ID is 0-65535');
                else
                    obj.atCommand('ID',varargin{1});
                end
            else
                obj.atCommand('ID',6975);
            end
            
            if obj.isCoordinator
                disp('This XBee connected to USB/Serial is a Coordinator.');
            else
                disp('This XBee connected to USB/Serial is a Router.');
                obj.atCommand('JV',1);
            end
            
            % Write the configuration created above to the firmware. This effectively
            % saves the configuration you have created to the XBee.
            obj.atCommand('WR');
            
        end
        
        function [output, sources] = readDigitalPin(obj, pin, varargin)
            %READDIGITALPIN Reads the digital value from the pin of the XBee object (obj).
            %   readDigitalPin reads the value (1/0 or true/false) of a digital
            %   pin of the XBee module(s) in the network.
            %
            %   readDigitalPin(x,19) reads the digital value of pin (19) of all the
            %   nodes in the XBee network as the default destination address is broadcast.
            %   This is useful only if there are few XBee modules deployed in the network.
            %
            %   readDigitalPin(x,4,'0013A200408BAE30') reads the digital value of pin (4) of
            %   the remote XBee module with the 64-bit address of '0013A200408BAE30'.
            
            % Check the number of arguments
            minargs = 2;
            maxargs = 3;
            narginchk(minargs, maxargs);
                        
            if nargin == 2
                destadd = '000000000000FFFF';
            else
                destadd = varargin{1};
            end
            
            if numel(pin)>1
                error('xbee:readDigitalPin:numPin', 'Please specify only one pin number.');
            end

            idx = find(obj.ConfigurableDigitalPins == pin, 1);
            if isempty(idx)
                error('xbee:readDigitalPin:pinChk', 'Pin %d unavailable.',pins);
            end
            
            [curPinMode, sources] = obj.getPinMode(pin, destadd);
            
            for s = 1:numel(sources)
                if strcmpi(curPinMode{s}, 'disabled') || isempty(curPinMode{s})
                    % if pin is either 'disabled' or there is an error while attempting to
                    % find pin mode, set pin mode to digitalInput.
                    setPinMode(obj, pin, 'digitalInput', sources{s});
                elseif ~strcmpi(curPinMode{s}, 'digitalInput')
                    % If the user has explicitly set the pin mode to be
                    % something other than 'digitalInput', then
                    % generate an error.
                    error('xbee:readDigitalPin:ModeMmatch', ...
                        'Current pin %d mode on module %s is ''%s'' but ''digitalInput'' is required.\nUse setPinMode() function to set pin mode to ''digitalInput''.', pin, sources{s}, curPinMode{s});
                end
            end
            
            responseIS = obj.remoteAtCommand('IS', destadd);
            
            numresponses = numel(responseIS);
            output = [];
            sources = cell(numresponses, 1);
            
            for r = 1:numresponses
                try
                    sources{r} = responseIS(r).srcAddress64;
                    
                    idx  = find(responseIS(r).sampleATResponse.digitalInputPins == pin, 1);
                    output(r) = responseIS(r).sampleATResponse.digitalInputPinValues(idx);
                    
                    if isempty(idx)
                        warning('xbee:readDigitalPin:EmptyIDX', 'Output of pin %d is unavailable from module %s.', pin, sources{r});
                    else
                        if numel(idx) > 2
                            warning('xbee:readDigitalPin:LengthChk', 'Length of returned data is too long.');
                        end
                    end                    
                catch
                    warning off backtrace;
                    warning('xbee:readDigitalPin:TryCtch', 'Output of your pin is unavailable.');
                    warning on backtrace;
                end
            end
            output = output(:);
        end
        
        function [output, sources] = readVoltage(obj, pin, varargin)
            %READVOLTAGE Reads the voltage from an analog pin of the XBee object (obj)
            %   readVoltage reads the voltage of an analog pin (ranging
            %   from 0 V to 1.2 V) of the XBee module(s) in the network.
            %
            %   readVoltage(x,19) reads the voltage of pin (19) of all the nodes in the
            %   XBee network as the default destination address is broadcast. This is
            %   useful only if there are few XBee modules deployed in the network.
            %
            %   readVoltage(x,4,'0013A200408BAE30') reads the voltage of pin (4) of
            %   the remote XBee module with the 64-bit address of '0013A200408BAE30'.
            
            
            % Check the number of arguments
            minargs = 2;
            maxargs = 3;
            narginchk(minargs, maxargs);
            
            if numel(pin)>1
                error('xbee:readVoltage:numPin', 'Please specify only one pin number.');
            end
            
            if nargin == 2
                destadd = '000000000000FFFF';
            else
                destadd = varargin{1};
            end
            
            
            idx = find(obj.ConfigurableAnalogPins == pin, 1);
            if isempty(idx)
                error('xbee:readVoltage:pinChk', 'Pin %d unavailable.',pin);
            end
            
            [curPinMode, sources] = obj.getPinMode(pin, destadd);
            
            for s = 1:numel(sources)
                if strcmpi(curPinMode{s}, 'disabled') || isempty(curPinMode{s})
                    % if pin is either 'disabled' or there is an error while attempting to
                    % find pin mode, set pin mode to analogInput.
                    setPinMode(obj, pin, 'analogInput', sources{s});
                elseif ~strcmpi(curPinMode{s}, 'analogInput')
                    % If the user has explicitly set the pin mode to be
                    % something other than 'analogInput', then generate
                    % an error.
                    error('xbee:readVoltage:ModeMmatch',...
                        'Current pin %d mode on module %s is ''%s'' but ''analogInput'' is required.\nUse setPinMode() function to set pin mode to ''analogInput''.', pin, sources{s}, curPinMode{s});
                end
            end
            
            
            responseIS = obj.remoteAtCommand('IS', destadd);
            
            numresponses = numel(responseIS);
            output = [];
            sources = cell(numresponses, 1);
            
            for r = 1:numresponses
                try
                    sources{r} = responseIS(r).srcAddress64;
                    
                    idx = find(responseIS(r).sampleATResponse.analogInputPins == pin, 1);
                    output(r) = responseIS(r).sampleATResponse.analogInputPinVoltage_V(idx);
                    
                    if isempty(idx)
                        warning('xbee:readVoltage:EmptyIDX', 'Output of pin %d is unavailable from module %s.', pin, sources{r});
                    else
                        if numel(idx) > 2
                            warning('xbee:readVoltage:LengthChk', 'Length of returned data is too long.');
                        end
                    end
                catch
                    warning off backtrace;
                    warning('xbee:readVoltage:TryCtch', 'Output of your pin is unavailable.');
                    warning on backtrace;
                end
            end
            
            output = output(:);
        end
        
        function [output, sources] = readSupplyVoltage(obj, varargin)
            %READVOLTAGE Reads the supply voltage from of the XBee object (obj)
            %   readSupplyVoltage reads the power supply voltage of the XBee module(s) in the network.
            %
            %   readSupplyVoltage(x) reads the supply voltage all the nodes in the
            %   XBee network as the default destination address is broadcast. This is
            %   useful only if there are few XBee modules deployed in the network.
            %
            %   readSupplyVoltage(x,'0013A200408BAE30') reads the supply voltage of 
            %   the remote XBee module with the 64-bit address of '0013A200408BAE30'.
            
            
            % Check the number of arguments
            minargs = 1;
            maxargs = 2;
            narginchk(minargs, maxargs);
            
            if nargin == 1
                destadd = '000000000000FFFF';
            else
                destadd = varargin{1};
            end
            
            responseIS = obj.remoteAtCommand('%V', destadd);
            
            numresponses = numel(responseIS);
            output = [];
            sources = cell(numresponses, 1);
            
            for r = 1:numresponses
                try
                    sources{r} = responseIS(r).srcAddress64;
                    output(r) = responseIS(r).sampleATResponse.supplyVoltage;
                catch
                    warning off backtrace;
                    warning('xbee:readSupplyVoltage:TryCtch', 'Supply voltage is unavailable.');
                    warning on backtrace;
                end
            end
            
            output = output(:);
        end
        
        function writeDigitalPin(obj, pin, value, varargin)
            %WRITEDIGITALPIN Write the digital pin value of the XBee object (obj)
            
            %   writeDigitalPin writes the pin value 1/0 or true/false to the XBee
            %   module(s) in the network.
            %
            %   writeDigitalPin(x, 19, 1) writes 1 (true) to the digital pin (19) of all
            %   the nodes in the XBee network as the default destination address is
            %   broadcast.
            %
            %   writeDigitalPin(x, 4, 0, '0013A200408BAE30') writes 0 (false) to the
            %   digital pin (4) of the remote XBee module with the 64-bit address of
            %   '0013A200408BAE30'.
            
            % Check the number of arguments
            minargs = 3;
            maxargs = 4;
            narginchk(minargs, maxargs);
            
            if numel(pin)>1
                error('xbee:writeDigitalPin:numPin', 'Please specify only one pin number.');
            end
            
            if nargin == 3
                destadd = '000000000000FFFF';
            else
                destadd = varargin{1};
            end
            
            idx = find(obj.ConfigurableDigitalPins == pin);
            if isempty(idx)
                error('xbee:writeDigitalPin:pinChk', 'Pin unavailable.');
            end
            command = cell2mat(obj.DigitalPinCmd(idx));
            
            [curPinMode, sources] = obj.getPinMode(pin, destadd);
            
            for s = 1:numel(sources)
                if strcmpi(curPinMode{s}, 'disabled')
                    % By default if the pin mode is disabled, then set it to digitalOutput
                    obj.setPinMode(pin, 'digitalOutput', sources{s});
                elseif ~strcmpi(curPinMode{s}, 'digitalOutput')
                    % If pin mode is set to either of these two options, generate error
                    error('xbee:writeDigitalPin:ModeMmatch', 'Current pin mode on module %s is ''%s'' but ''digitalOutput'' is required. Use setPinMode() function to set pin mode to ''digitalOutput''.', sources{s}, curPinMode{s});
                end
            end
            
            if value == 1
                % 5 signals the XBee to change state of pin to a digital high
                obj.remoteAtCommand(command, destadd, 5);
            elseif value == 0
                % 5 signals the XBee to change state of pin to a digital high
                obj.remoteAtCommand(command, destadd, 4);
            else
                error('xbee:writeDigitalPin:DigVal', 'Please use 0 or 1 for the digital output value.');
            end
        end
        
        
        function [output, sources] = setPinMode(obj, pin, mode, varargin)
            %SETPINMODE Set the pin mode of XBee object.
            %   setPinMode sets the pin mode of remote XBee module. There are 4
            %   possible modes: 'disabled', 'analogInput', 'digitalOutput', or 'digitalInput'.
            %
            %   setPinMode(x, 19, 'analogInput') sets the pin (19) of all the nodes in the
            %   XBee network as analog input. The default destination address is
            %   broadcast.
            %
            %   setPinMode(x, 4, 'digitalOutput', '0013A200408BAE30') sets the pin (19) of
            %   the remote XBee module with the 64-bit address of '0013A200408BAE30'
            %   as digital output.
            
            % Check the number of arguments
            minargs = 3;
            maxargs = 4;
            narginchk(minargs, maxargs);
            
            if nargin == 3
                destadd = '000000000000FFFF';
            else
                destadd = varargin{1};
            end
            
            
            idx = find(obj.ConfigurableDigitalPins == pin);
            if isempty(idx)
                error('xbee:setPinMode:pinChk', 'Pin unavailable.')
            end
            command = cell2mat(obj.DigitalPinCmd(idx));
            
            switch lower(mode)
                case {'disabled'}
                    val = 0;
                case {'analoginput', 'analogin'}
                    val = 2;
                case {'digitalout', 'digitaloutput'}
                    val = 4;
                case {'digitalinput', 'digitalin'}
                    val = 3;
                otherwise
                    error('xbee:setPinMode:ModeErr', 'Unexpected mode, only the following modes {disabled, analogInput, digitalOutput, digitalInput} are supported.');
            end
            
            responses = obj.remoteAtCommand(command, destadd, val);
            
            if nargout == 1
                output  = [responses(:).state]';
            elseif nargout == 2
                output  = [responses(:).state]';
                sources = responses(:).srcAddress64;
            end
        end
        
        function [mode, sources] = getPinMode(obj, pin, varargin)
            %GETPINMODE get the pin mode of the XBee object.
            %   getPinMode gets the pin mode of remote XBee module. There are 4 types of
            %   different pin modes: 'disabled', 'analogInput', 'digitalOutput', or
            %   'digitalInput'.
            %
            %   getPinMode(x,19) gets the pin (19) of all the nodes in the
            %   XBee network as the default destination address is broadcast. This is
            %   useful only if there are few XBee modules deployed in the network.
            %
            %   getPinMode(x,4,'0013A200408BAE30') gets the pin (19) of
            %   the remote XBee module with the 64-bit address of '0013A200408BAE30'.
            
            
            % Check the number of arguments
            minargs = 2;
            maxargs = 3;
            narginchk(minargs, maxargs);
            
            if nargin == 2
                destadd = '000000000000FFFF';
            else
                destadd = varargin{1};
            end
            
            idx=find(obj.ConfigurableDigitalPins == pin);
            
            if isempty (idx)
                error('xbee:getPinMode:pinChk', 'Pin unavailable.');
            end
            command = cell2mat(obj.DigitalPinCmd(idx));
            
            responses = obj.remoteAtCommand(command, destadd);
            
            numresponses = numel(responses);
            mode = cell(numresponses,1);
            sources = cell(numresponses, 1);
            
            for r = 1:numresponses
                sources{r} = responses(r).srcAddress64;
                if responses(r).state == 0
                    switch responses(r).sampleATResponseRaw
                        case {0, 1}
                            mode{r} = 'disabled';
                        case 2
                            mode{r} = 'analogInput';
                        case 3
                            mode{r} = 'digitalInput';
                        case {4, 5}
                            mode{r} = 'digitalOutput';
                        otherwise
                            warning('xbee:getPinMode:SwitchOther', 'Unexpected mode, check your mode {disabled, analogInput, digitalInput, digitalOutput}');
                            mode{r} = response.sampleATResponseRaw;
                    end
                else
                    warning off backtrace;
                    warning('xbee:getPinMode:MsgCorrupt', 'Message corrupted therefore discarding.');
                    warning on backtrace;
                end
            end
        end
        
        function output = isCoordinator(obj)
            %ISCOORDINATOR Check whether the USB/Serial connected XBee module is a
            % Coordinator or Router and return 1 if the XBee module is a
            % coordinator.
            
            temp = obj.atCommand('VR');
            if temp(1) == hex2dec('21')
                output = 1;
            else
                output = 0;
            end
        end
        
        
        function varargout = getLocalParameters(obj)
            %GETLOCALPARAMETERS provides the configuration information
            %   getLocalParameters returns the following values in the described order:
            %   1. PANID,
            %   2. Operating PANID,
            %   3. Address64
            %   4. Address16
            %   5. BaudRate
            %   6. FirmwareVersion
            %   7. HardwareVersion
            
            temp = obj.atCommand('ID');
            temp = dec2hex(temp,2);
            PANID = reshape(temp', 1, numel(temp));
            
            temp = obj.atCommand('OP');
            temp = dec2hex(temp,2);
            OPANID = reshape(temp', 1, numel(temp));
            
            % Extract 64-bit source address of the packet
            SH = dec2hex(obj.atCommand('SH'),2);
            SL = dec2hex(obj.atCommand('SL'),2);
            temp= [SH; SL];
            Address64 = reshape(temp', 1, numel(temp));
            
            % Extract 16-bit source address of the packet
            temp = dec2hex(obj.atCommand('MY'),2);
            Address16 = reshape(temp', 1, numel(temp));
            
            temp = obj.atCommand('BD');
            temp = temp(temp ~= 0);
            switch temp
                case 0
                    BaudRate = 1200;
                case 1
                    BaudRate = 2400;
                case 2
                    BaudRate = 4800;
                case 3
                    BaudRate = 9600;
                case 4
                    BaudRate = 19200;
                case 5
                    BaudRate = 38400;
                case 6
                    BaudRate = 57600;
                case 7
                    BaudRate = 115200;
                otherwise
                    error('xbee:getLocalParams:Otherwise', 'Instrument Data rate is not supported.');
            end
            
            temp = dec2hex(obj.atCommand('VR'),2);
            FirmwareVersion = reshape(temp', 1, numel(temp));
            
            temp = dec2hex(obj.atCommand('HV'),2);
            HardwareVersion = reshape(temp', 1, numel(temp));
            
            if nargout == 0
                parameters = sprintf([...
                    '\n           PANID: %s',...
                    '\n Operating PANID: %s',...
                    '\n       Address64: %s',...
                    '\n       Address16: %s',...
                    '\n        BaudRate: %d',...
                    '\n FirmwareVersion: %s',...
                    '\n HardwareVersion: %s\n'],...
                    PANID, OPANID, Address64, Address16,...
                    BaudRate, FirmwareVersion, HardwareVersion);
                disp(parameters);
            else
                varargout{1} = PANID;
                varargout{2} = OPANID;
                varargout{3} = Address64;
                varargout{4} = Address16;
                varargout{5} = BaudRate;
                varargout{6} = FirmwareVersion;
                varargout{7} = HardwareVersion;                
            end
        end
        
    end
    
    
    % Define private properties
    properties (Access = private)
        % Container for the serial object
        Serial;
        % SIFS (Short Inter Frame Space) is the period for the nodes to
        % defer using the medium while waiting for a response.
        SIFS;
    end
    
    % Define constant and private properties
    properties (Constant = true, GetAccess = private)
        AnalogPinCmd   = {'D3', 'D2', 'D1', 'D0'};
        DigitalPinCmd  = {'P2', 'P0', 'P1', 'D4', 'D7', 'D5', 'D6',...
            'D3', 'D2', 'D1', 'D0'};
        ModulePinNames = [12 10 11 4 7 5 6 3 2 1 0];
    end
    
    % Define hidden methods
    methods (Hidden = true)
        
        function obj = xbee(serialPort)
            obj.Serial = serial(serialPort);
            try
                fopen(obj.Serial);
            catch eConnErr
                throwAsCaller(eConnErr);
            end
            obj.SIFS   = 1.5;
            obj.Port   = obj.Serial.Port;
            obj.BaudRate = obj.Serial.BaudRate;
        end
        
        function delete(obj)
            fclose(obj.Serial);
            delete(obj.Serial);
            obj.Serial = [];
        end
        
        % Hide functions inherited from the handle class
        function addlistener(obj, property, eventname, callback)
            addlistener@addlistener(obj, property, eventname, callback)
        end
        
        function findobj(obj, property, eventname, callback)
            findobj@findobj(obj, property, eventname, callback)
        end
        
        function findprop(obj, property, eventname, callback)
            findprop@findprop(obj, property, eventname, callback)
        end
        
        function notify(obj, property, eventname, callback)
            notify@notify(obj, property, eventname, callback)
        end
    end
    
    methods
        function set.BaudRate(obj, value)
            classes = {'numeric'};
            attributes = {'scalar'};
            validateattributes(value, classes, attributes);
            
            supportedBaudRate = [1200, 2400, 4800, 9600, 19200, 38400,...
                57600, 115200];
            
            if ismember(value, supportedBaudRate)
                obj.Serial.BaudRate = value;
            else
                suppRateArraySize = repmat('%d, ', 1, length(supportedBaudRate));
                errorString = sprintf(['%d is not a supported rate. Supported rates are: {' suppRateArraySize(1:end-2) '}, please provide a valid value.'], value, supportedBaudRate);
                error('xbee:BdRate', errorString);  %#ok<SPERR>
            end
        end
        
        function output = get.BaudRate(obj)
            output = obj.Serial.BaudRate;
        end
    end    
    
    methods (Hidden = true, Access = private)
        function output = remoteAtCommand(obj, command, destadd, varargin)
            % REMOTEATCOMMAND Send Remote AT Command to the remote node (destadd) and
            % returns structure of the Remote AT Command response.
            
            if obj.Serial.BytesAvailable ~= 0
                % If there are data queued in the Serial object, empty it
                flushinput(obj.Serial);
            end
            
            % Check the number of arguments
            minargs = 3;
            maxargs = 4;
            narginchk(minargs, maxargs);
            
            % Change 16 hexadecimal numbers to 8 decimal numbers.
            decDestAdd = hex16ToDec8(destadd);
            
            % Structure Remote AT Command frame (Frame type - 0x17)
            frame = [ ...
                hex2dec('7E'), ... % Start byte
                hex2dec('00'), hex2dec('00'), ... % Length: fill in later
                hex2dec('17'), ... % Frame type 0x17 is Remote AT command request
                hex2dec('77'), ... % Frame ID % This can be an identifier.
                decDestAdd,    ... % 64bit Destination Address
                hex2dec('FF'), hex2dec('FE'), ... % Destination Network Address
                hex2dec('02'), ... % Apply changes immediatly
                uint8(command(1)), ... % Command byte 1
                uint8(command(2)) ...  % Command byte 2
                ];
            
            % If the command comes with the value, include the value in the Remote
            % AT Command frame.
            if nargin == 4
                % the value to be assigned needs to be broken into two parts, VH (Value
                % High) and VL (Value Low) and then concatenated as two separate
                % entities of the frame
                VH = floor(varargin{1}/256);
                VL = mod(varargin{1}, 256);
                frame = [frame VH VL];
            end
            
            % Include frame length in the frame. Frame structure specs needs it to be
            % broken into two parts LH and LL
            frameLength = length(frame (4:end));
            LH = floor(frameLength/ 256);
            LL = mod(frameLength, 256);
            frame(2) = LH;
            frame(3) = LL;
            
            % Include checksum in the frame
            checksum = hex2dec('FF') - mod(sum(frame(4:end)), 256);
            frame = [frame checksum];
            
            % Send it to the XBee module
            fwrite(obj.Serial, frame);
            
            % Wait for some time (Short InterFrame Space:SIFS) for a remote node to
            % receive and process the message, and reply back to the local node with
            % the response.
            pause(obj.SIFS);
            
            % If there is no data received from the serial object after waiting for
            % some time, return the empty output.
            output = struct(...
                'srcAddress64',{},...
                'srcAddress16',{},...
                'ATCommand',{},...
                'state',{},...
                'sampleATResponse',{},...
                'sampleATResponseRaw',{});
            
            if obj.Serial.BytesAvailable == 0
                error('xbee:remoteAtCommand', ['No response from your Remote node. ', ...
                    'Check the destination address or SIFS parameter if you want to increase the waiting time.']);
            end
            
            % Process the response of the Remote AT Command. This functionality is
            % implemented in a WHILE loop because there may be backlogged frames in
            % the queue. One way would be to discard all the backlogged frames, but
            % it has been designed to indentify all the backlogged data.
            while obj.Serial.BytesAvailable ~= 0
                if fread(obj.Serial, 1) == hex2dec('7E') % Start delimeter
                    
                    % Read the first byte (The first byte of Length of the frame)
                    % Check ByteAvailable before reading Bytes
                    if obj.Serial.BytesAvailable ~= 0
                        LH = fread(obj.Serial, 1);
                    else
                        error('xbee:remoteAtCommand:BytesAvail', 'No data available in Serial.');
                    end
                    
                    % If there is duplicated start delimeter, discard it
                    if LH == hex2dec('7E')
                        continue;
                    else
                        if obj.Serial.BytesAvailable ~= 0
                            % Read the second byte (of Length of the frame)
                            LL = fread(obj.Serial, 1);
                        else
                            error('xbee:remoteAtCommand:BytesAvail', 'No data available in Serial.');
                        end
                        
                        % Extract frame length from the packet
                        frameLength = LH * 256 + LL;
                        
                        % Read the command specific data from the frame. If the frame
                        % length (bytes) identified in the frame is more than the actual
                        % bytes received, generate an error
                        if frameLength < obj.Serial.BytesAvailable
                            dataFrame = fread(obj.Serial, frameLength);
                        else
                            error('xbee:remoteAtCommand:FullFrame', 'The full frame has not been received yet');
                        end
                        
                        if obj.Serial.BytesAvailable ~= 0
                            % Read next byte (checksum) and discard it.
                            fread(obj.Serial, 1);
                        else
                            error('xbee:remoteAtCommand:ChkSumErr', 'There is no Checksum. Packet framing error.');
                        end
                        
                        % Process the command specific dataframe (bytes) in
                        % extractFrameData function to extract information.
                        latestFrame = extractFrameData(dataFrame, command);
                        if(~isempty(latestFrame))
                            output(end+1) = latestFrame; %#ok<AGROW>
                        end
                    end
                else
                    warning('xbee:remoteAtCommand:Delim', 'Current byte is not a start delimeter. Discarding it');
                end
            end
            
            % Waiting a short period to allow for remote instrument AT command to be
            % processed by Xbee module. Adjust this value as needed.
            waitForCompletion = 0.5; %seconds
            pause(waitForCompletion);            
        end
        
        function output = atCommand(obj, command, varargin)
            % ATCOMMAND Send AT Command to the Serial/USB connected XBee module and
            % returns the Remote AT Command response.
            
            % If there is data queued in the Serial object, empty it.
            if obj.Serial.BytesAvailable ~= 0
                % If data is already available in the Serial object, empty it
                flushinput(obj.Serial);
            end
            
            % Check the number of arguments
            minargs = 2;
            maxargs = 3;
            narginchk(minargs, maxargs);
            
            if length(command) ~= 2
                error('xbee:atCommand:argChk', 'The size of AT command should be two bytes.')
            end
            
            frame = [ ...
                hex2dec('7E'), ... % Start byte
                hex2dec('00'), hex2dec('00'), ... % Length: fill in later
                hex2dec('08'), ... % Frame type 0x078 is AT command request
                hex2dec('52'), ... % Frame ID
                uint8(command(1)), ...
                uint8(command(2)) ...
                ];
            
            % If the command comes with the value, include the value in the Remote
            % AT Command frame.
            if nargin == 3
                VH = floor(varargin{1}/256);
                VL = mod(varargin{1}, 256);
                frame = [frame VH VL];
            end
            
            frameLength = length(frame(4:end));
            
            % Length calculation
            LH = floor(frameLength/256);
            LL = mod(frameLength, 256);
            frame(2) = LH;
            frame(3) = LL;
            
            % Calculate checksum
            checksum = hex2dec('FF') - mod(sum(frame(4:end)), 256);
            frame = [frame checksum];
            
            % Send it to the XBee module
            fwrite(obj.Serial, frame);
            
            
            % Wait for some time for the node to receive and process the
            % message, and reply back with the response.
            if strcmpi(command, 'ND')
                % We have to wait longer for Node Discovery since all
                % the network's nodes responses have to be received and
                % collected.
                pause(obj.SIFS*5);
            else
                % Wait till the written command is processed by the module
                waitTillCompletion = 0.2;
                pause(waitTillCompletion);
            end

            % Initialize the output variable.
            output = {};
            
            if obj.Serial.BytesAvailable == 0
                error('xbee:atCommand:ZeroBytes', 'Zero bytes available to read.');
            end
            
            % Process the response to the AT Command. This functionality is
            % implemented in a WHILE loop because there may be multiple
            % responses to the AT command, in particular, the ND command.
            while obj.Serial.BytesAvailable ~= 0
                if fread(obj.Serial, 1) == hex2dec('7E') % Start delimeter
                    
                    % Read the first byte (The first byte of Length of the frame)
                    % Check ByteAvailable before reading Bytes
                    if obj.Serial.BytesAvailable ~= 0
                        LH = fread(obj.Serial, 1);
                    else
                        error('xbee:atCommand:BytesAvail', 'No data available in Serial.');
                    end
                    
                    if obj.Serial.BytesAvailable ~= 0
                        % Read the second byte (of Length of the frame)
                        LL = fread(obj.Serial, 1);
                    else
                        error('xbee:atCommand:BytesAvail', 'No data available in Serial.');
                    end
                    
                    % Extract frame length from the packet
                    frameLength = LH * 256 + LL;
                    
                    % Read the command specific data from the frame. If the frame
                    % length (bytes) identified in the frame is more than the actual
                    % bytes received, generate an error
                    if frameLength < obj.Serial.BytesAvailable
                        atCmdRspFrame = fread(obj.Serial, frameLength);
                    else
                        error('xbee:atCommand:FullFrame', 'The full frame has not been received yet');
                    end
                    
                    if obj.Serial.BytesAvailable ~= 0
                        % Read next byte (checksum) and discard it.
                        fread(obj.Serial, 1);
                    else
                        error('xbee:atCommand:ChkSumErr', 'There is no Checksum. Packet framing error.');
                    end
                    
                    % Command status
                    switch (atCmdRspFrame(5))
                        % 0 is the expected value
                        case 1
                            warning('xbee:atCommand:CommandStatus1', 'Error returned from coordinator module.')
                        case 2
                            warning('xbee:atCommand:CommandStatus2', 'Invalid command');
                        case 3
                            warning('xbee:atCommand:CommandStatus3', 'Invalid Parameter');
                        case 4
                            warning('xbee:atCommand:CommandStatus4', 'Transmission Failure');
                        otherwise
                            % something is wrong if none of the above cases are satisfied. We
                            % will not throw a warning for this case in this version
                    end
            
                    if (length(atCmdRspFrame) > 5)
                        % there is a variable in the response
                        output{end+1} = atCmdRspFrame(6:end);
                    end
                end
            end
            
            % Unless we are dealing with the 'ND' command, return only the
            % first response (there should only be one anyway).
            if ~strcmpi(command, 'ND')
                output = output{1};
            end
        end
    end
end

%%

function output = extractFrameData(frameData, remoteATCommand)
% EXTRACTFRAMEDATA Extract the data (Address, Frame, ID, Digital/Analog IO)
% from the frame.

    output = [];
    if ~strcmp (remoteATCommand, char(frameData (13:14))')
        error('xbee:extractFrameData:CharChk', 'This is not the response of the expected AT command');
    end

    if length(frameData) < 15
        error('xbee:extractFrameData:LengthChk', 'Number of bytes is not enough to extract info');
    end

    % Extract the frame type
    frameType = frameData(1);

    % Based on the frame type, process the frame differently.
    switch (frameType)
        % 0x97 is a frame type of [AT Remote Command Response]
        case hex2dec('97')

            frameID = frameData(2);
            if frameID ~= hex2dec('77')
                error('xbee:extractFrameData:Case97', 'This command is not from MATLAB');
            end

            % Extract 64-bit source address of the packet
            temp = dec2hex(frameData(3:10),2);
            output.srcAddress64 = reshape(temp', 1, numel(temp));

            % Extract 16-bit source address of the packet
            temp = dec2hex(frameData(11:12),2);
            output.srcAddress16 = reshape(temp', 1, numel(temp));

            % Extract Remote AT Command
            output.ATCommand = char(frameData(13:14))';

            warning off backtrace
            % Extract Remote AT Command Status
            switch (frameData(15))
                case 0
                    %CommandStatus: Sent successfully
                    output.state = 0;
                case 1
                    warning('xbee:extractFrameData:Case1', 'Command ERROR! Check your command')
                    output.state = 1;
                case 2
                    warning('xbee:extractFrameData:Case2', 'Invalid Command')
                    output.state = 2;
                case 3
                    warning('xbee:extractFrameData:Case3', 'Invalid Parameter')
                    output.state = 3;
                case 4
                    warning('xbee:extractFrameData:Case4', 'Transmission Failure')
                    output.state = 4;
                otherwise
                    warning('xbee:extractFrameData:Case5','CommandStatus: Error but continuing.')
            end
            warning on backtrace
            % if the frameData is more than 15 bytes, which is the minimum
            % bytes of Remote AT Response frame, it includes ADIO data.
            if (length(frameData) > 15)
                % Extract Analog and Digital input output (ADIO) values from the
                % byte data and return the output.
                if strcmp(remoteATCommand,'IS')
                    output.sampleATResponse = extractCmdData(frameData(16:end)');
                elseif strcmp(remoteATCommand,'%V')
                    rawvalue = frameData(16)*256+frameData(17);
                    output.sampleATResponse.supplyRawValue = rawvalue;
                    output.sampleATResponse.supplyVoltage = rawvalue*1.2/1024;
                else
                    output.sampleATResponse = [];
                end

                % Return the raw data frame for debugging
                output.sampleATResponseRaw = frameData(16:end)';
            else
                output.sampleATResponse = [];
                output.sampleATResponseRaw = [];

            end

            % List of different frame types
        case hex2dec('88')
            disp('This is an AT Command Response');
        case hex2dec('8A')
            disp('This is a Modem Status');
        case hex2dec('8B')
            disp('This is a ZigBee Transmit Status');
        case hex2dec('90')
            disp('This is a ZigBee Receive Packet (AO=0)');
        case hex2dec('91')
            disp('This is a ZigBee Explicit Rx Indicator (AO=1)');
        case hex2dec('92')
            disp('This is a ZigBee IO Data Sample Rx Indicator');
        case hex2dec('94')
            disp('This is an XBee Sensor Read Indicator (AO=0)');
        case hex2dec('95')
            disp('This is a Node Identification Indicator (AO=0)');
        otherwise
            disp('This is an unidentifiable framedata');
            return;
    end



    function output = extractCmdData(rawCmdData)
        % EXTRACTCMDDATA is used to extract the data payload in the frame
        % returned from the XBee module.
        
        if length(rawCmdData) < 6
            output = 0;
            warning('xbee:extractFrameData:extractCmdData:RawLength', 'No output pins available');
            return;
        end
        
        % Return number of sample set
        output.numSampleSet = rawCmdData(1);
        
        % Return digital input pin numbers
        digitalPinBinMSB   = fliplr(dec2bin(rawCmdData(2),8));
        digitalInputPinMSB = find(digitalPinBinMSB=='1') + 7;
        digitalPinBinLSB   = fliplr(dec2bin(rawCmdData(3),8));
        digitalInputPinLSB = find(digitalPinBinLSB=='1')-1;
        digitalInputPins   = [digitalInputPinMSB digitalInputPinLSB];
        
        % Return analog input pin numbers
        analogPin = fliplr(dec2bin(rawCmdData(4),8));
        
        analogInputPins = 21-find(analogPin=='1');
        
        %% Sanity check for the number of bytes for digital and analog inputs
        numDigitalInput = ~isempty(digitalInputPins);
        numAnalogInput  = length(analogInputPins);
        estTotBytes     = 1 + 2 + 1 + numDigitalInput*2 + numAnalogInput*2;
        actualBytes     = length(rawCmdData);
        
        if estTotBytes ~= actualBytes
            warning('xbee:extractFrameData:extractCmdData:BytesMmatch', 'Data returned is not of expected length, discarding.');
            output = 0;
        end
        
        % If digital input pin bytes are NOT empty, read digital input values
        digitalInputPinValues = [];
        analogInputPinValues = [];
        if ~isempty(digitalInputPins)
            % DIGITAL Input found
            digitalPinValueBinMSB = fliplr(dec2bin(rawCmdData(5),8));
            digitalInputPinValueMSB = digitalPinValueBinMSB(digitalInputPinMSB-7);
            digitalPinValueBinLSB = fliplr(dec2bin(rawCmdData(6),8));
            digitalInputPinValueLSB = digitalPinValueBinLSB(digitalInputPinLSB+1);
            digitalInputPinValues = [digitalInputPinValueMSB digitalInputPinValueLSB];
            
            % If analog input pin bytes are NOT empty, read analog input
            % values as well
            if ~isempty(analogInputPins)
                % ANALOG Input found
                for i = 1:numAnalogInput
                    AnalogPinValueBinMSB = rawCmdData( (i-1)*2+ 7);
                    AnalogPinValueBinLSB = rawCmdData( (i-1)*2+ 8);
                    analogInputPinValues = [analogInputPinValues AnalogPinValueBinMSB*256 + AnalogPinValueBinLSB];  %#ok<AGROW>
                end
            else
                % Only digital
                analogInputPins = [];
            end            
        else
            % There are no digital inputs
            digitalInputPins = [];
            
            % If analog input pin bytes are NOT empty, read analog input
            % values
            if ~isempty(analogInputPins)
                % Only analog
                for i = 1:numAnalogInput
                    AnalogPinValueBinMSB = rawCmdData( (i-1)*2+ 5);
                    AnalogPinValueBinLSB = rawCmdData( (i-1)*2+ 6);
                    analogInputPinValues = [analogInputPinValues AnalogPinValueBinMSB*256 + AnalogPinValueBinLSB]; %#ok<AGROW>
                end
            else
                warning('xbee:extractFrameData:extractCmdData:AnalogEmpty', 'There are no bytes at the specified analog input pins.');
            end
        end
        
        % Convert ModulePinNames to actual DigitalPin number
        tempVariable = digitalInputPins;
        if ~isempty(tempVariable)
            digitalInputPins = [];
            for i = 1:length(tempVariable)
                idx = find(xbee.ModulePinNames == tempVariable(i));
                digitalInputPins = [digitalInputPins xbee.ConfigurableDigitalPins(idx)];  %#ok<AGROW,FNDSB>
            end
        end
        
        % Return outputs in a structure.
        output.digitalInputPins        = digitalInputPins;
        output.digitalInputPinValues   = double(digitalInputPinValues)-48; % formatting to double number
        output.analogInputPins         = analogInputPins;
        output.analogInputPinValues    = analogInputPinValues;
        output.analogInputPinVoltage_V = (analogInputPinValues * 1200 / 1023)/1000;
        
    end
end

function output = hex16ToDec8(hex16Value)
% HEX16TODEC8 Convert 16 char hexadecimal numbers to 8 digit decimal numbers.
%
%   xbee.hex16ToDec8('0013A200409BAE30') returns [ 0 19 162 0 64 155 174 48].


str = upper(hex16Value);
temp = zeros(1,8);
if (length(str) ~= 16)
    error('hex16ToDec8:LengthMmatch', 'Check the size of string format. destination address should be 64bit HW address (e.g., ''0013A200408BAE30'')');
else
    for i = 1:length(str)/2
        x = [str( (i-1)*2 + 1) str( (i-1)*2 + 2)];
        temp(i) = hex2dec(x);
    end
end
output = temp;
end