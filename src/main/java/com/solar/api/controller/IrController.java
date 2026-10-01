package com.solar.api.controller;

import com.solar.api.model.IrCommand;
import com.solar.api.model.IrDevice;
import com.solar.api.model.IrDevice.IrMode;
import com.solar.api.service.IrService;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;
import java.util.Map;

@RestController
@RequestMapping("/api/ir")
@CrossOrigin(origins = "*")
public class IrController {

    private final IrService service;

    public IrController(IrService service) {
        this.service = service;
    }

    // ESP32 polls: [{"id":1,"name":"AC","mode":"MODE2","rawCode":"0000 006D ..."},...]
    // rawCode is null if no command registered for the current mode yet
    @GetMapping
    public ResponseEntity<List<IrService.IrDeviceState>> getAllStates() {
        return ResponseEntity.ok(service.getAllStates());
    }

    // Frontend: PUT /api/ir/1  {"mode":"MODE2"}
    @PutMapping("/{deviceId}")
    public ResponseEntity<IrDevice> setMode(
            @PathVariable int deviceId,
            @RequestBody Map<String, String> body) {
        IrMode mode = IrMode.valueOf(body.get("mode").toUpperCase());
        return ResponseEntity.ok(service.setMode(deviceId, mode));
    }

    // Register raw IR code: POST /api/ir/1/command  {"mode":"MODE1","rawCode":"0000 006D ..."}
    @PostMapping("/{deviceId}/command")
    public ResponseEntity<IrCommand> saveCommand(
            @PathVariable int deviceId,
            @RequestBody Map<String, String> body) {
        IrMode mode = IrMode.valueOf(body.get("mode").toUpperCase());
        String rawCode = body.get("rawCode");
        return ResponseEntity.ok(service.saveCommand(deviceId, mode, rawCode));
    }
}
