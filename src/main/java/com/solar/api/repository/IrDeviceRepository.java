package com.solar.api.repository;

import com.solar.api.model.IrDevice;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

@Repository
public interface IrDeviceRepository extends JpaRepository<IrDevice, Integer> {
}
