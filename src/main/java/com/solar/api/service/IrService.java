package com.solar.api.service;

import com.solar.api.model.IrCommand;
import com.solar.api.model.IrDevice;
import com.solar.api.model.IrDevice.IrMode;
import com.solar.api.repository.IrCommandRepository;
import com.solar.api.repository.IrDeviceRepository;
import jakarta.annotation.PostConstruct;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class IrService {

    private final IrDeviceRepository deviceRepo;
    private final IrCommandRepository commandRepo;

    public IrService(IrDeviceRepository deviceRepo, IrCommandRepository commandRepo) {
        this.deviceRepo = deviceRepo;
        this.commandRepo = commandRepo;
    }

    @PostConstruct
    public void init() {
        seedDevice("Kitchen Light",  "Kitchen",  IrDevice.DeviceType.ON_OFF);
        seedDevice("Living AC",      "Living",   IrDevice.DeviceType.AC);
        seedDevice("Living TV",      "Living",   IrDevice.DeviceType.ON_OFF);
        seedDevice("Living Amp",     "Living",   IrDevice.DeviceType.ON_OFF);
        seedDevice("Workroom Light", "Workroom", IrDevice.DeviceType.ON_OFF);
    }

    private void seedDevice(String name, String location, IrDevice.DeviceType deviceType) {
        boolean exists = deviceRepo.findAll().stream()
                .anyMatch(d -> d.getName().equals(name));
        if (!exists) {
            IrDevice device = new IrDevice();
            device.setName(name);
            device.setLocation(location);
            device.setDeviceType(deviceType);
            deviceRepo.save(device);
        }
    }

    // Returns all devices with their current mode and the raw code for that mode (null if not registered yet)
    public List<IrDeviceState> getAllStates() {
        return deviceRepo.findAll().stream().map(device -> {
            String code = commandRepo.findByDeviceAndMode(device, device.getMode())
                    .map(IrCommand::getRawCode)
                    .orElse(null);
            return new IrDeviceState(device.getId(), device.getName(), device.getLocation(),
                    device.getDeviceType(), device.getMode(), code);
        }).toList();
    }

    public IrDevice setMode(int deviceId, IrMode mode) {
        IrDevice device = deviceRepo.findById(deviceId)
                .orElseThrow(() -> new IllegalArgumentException("IR device not found: " + deviceId));
        device.setMode(mode);
        return deviceRepo.save(device);
    }

    public IrCommand saveCommand(int deviceId, IrMode mode, String rawCode) {
        IrDevice device = deviceRepo.findById(deviceId)
                .orElseThrow(() -> new IllegalArgumentException("IR device not found: " + deviceId));
        IrCommand cmd = commandRepo.findByDeviceAndMode(device, mode)
                .orElseGet(IrCommand::new);
        cmd.setDevice(device);
        cmd.setMode(mode);
        cmd.setRawCode(rawCode);
        return commandRepo.save(cmd);
    }

    public record IrDeviceState(int id, String name, String location, IrDevice.DeviceType deviceType, IrMode mode, String rawCode) {}
}
