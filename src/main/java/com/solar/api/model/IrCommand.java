package com.solar.api.model;

import jakarta.persistence.*;

@Entity
@Table(name = "ir_command", uniqueConstraints = {
        @UniqueConstraint(columnNames = {"device_id", "mode"})
})
public class IrCommand {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Integer id;

    @ManyToOne(optional = false)
    @JoinColumn(name = "device_id")
    private IrDevice device;

    @Enumerated(EnumType.STRING)
    @Column(nullable = false)
    private IrDevice.IrMode mode;

    // Raw IR signal stored as hex string, e.g. "0000 006D 0022 0002 ..."
    @Column(nullable = false, length = 2048)
    private String rawCode;

    public Integer getId() { return id; }
    public IrDevice getDevice() { return device; }
    public void setDevice(IrDevice device) { this.device = device; }
    public IrDevice.IrMode getMode() { return mode; }
    public void setMode(IrDevice.IrMode mode) { this.mode = mode; }
    public String getRawCode() { return rawCode; }
    public void setRawCode(String rawCode) { this.rawCode = rawCode; }
}
