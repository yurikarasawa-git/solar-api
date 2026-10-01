package com.solar.api.controller;

import com.solar.api.model.RelayState;
import com.solar.api.service.RelayService;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;
import java.util.Map;

@RestController
@RequestMapping("/api/relay")
@CrossOrigin(origins = "*")
public class RelayController {

    private final RelayService service;

    public RelayController(RelayService service) {
        this.service = service;
    }

    // ESP32 polls: GET /api/relay?location=2F
    @GetMapping
    public ResponseEntity<List<RelayState>> getAllStates(
            @RequestParam(required = false) String location) {
        return ResponseEntity.ok(service.getAllStates(location));
    }

    // Frontend: PUT /api/relay/2  { "state": true }
    @PutMapping("/{relayNumber}")
    public ResponseEntity<RelayState> setState(
            @PathVariable int relayNumber,
            @RequestBody Map<String, Boolean> body) {
        return ResponseEntity.ok(service.setState(relayNumber, body.get("state")));
    }
}
