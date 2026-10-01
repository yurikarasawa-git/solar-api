package com.solar.api.repository;

import com.solar.api.model.IrCommand;
import com.solar.api.model.IrDevice;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

import java.util.Optional;

@Repository
public interface IrCommandRepository extends JpaRepository<IrCommand, Integer> {
    Optional<IrCommand> findByDeviceAndMode(IrDevice device, IrDevice.IrMode mode);
}
