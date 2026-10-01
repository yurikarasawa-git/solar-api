package com.solar.api.model;

import jakarta.persistence.*;
import java.time.LocalDateTime;

@Entity
@Table(name = "ir_device")
public class IrDevice {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Integer id;

    @Column(nullable = false, unique = true)
    private String name;

    @Column(nullable = false)
    private String location;

    @Enumerated(EnumType.STRING)
    @Column(nullable = false)
    private DeviceType deviceType = DeviceType.ON_OFF;

    @Enumerated(EnumType.STRING)
    @Column(nullable = false)
    private IrMode mode = IrMode.OFF;

    private LocalDateTime updatedAt;

    @PrePersist
    @PreUpdate
    protected void onUpdate() {
        updatedAt = LocalDateTime.now();
    }

    public enum IrMode {
        OFF, MODE1, MODE2, MODE3, MODE4
    }

    public enum DeviceType {
        ON_OFF, AC
    }

    public Integer getId() { return id; }
    public String getName() { return name; }
    public void setName(String name) { this.name = name; }
    public String getLocation() { return location; }
    public void setLocation(String location) { this.location = location; }
    public DeviceType getDeviceType() { return deviceType; }
    public void setDeviceType(DeviceType deviceType) { this.deviceType = deviceType; }
    public IrMode getMode() { return mode; }
    public void setMode(IrMode mode) { this.mode = mode; }
    public LocalDateTime getUpdatedAt() { return updatedAt; }
}
