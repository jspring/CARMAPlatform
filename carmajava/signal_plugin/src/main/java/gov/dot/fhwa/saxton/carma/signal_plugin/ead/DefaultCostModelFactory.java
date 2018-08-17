/*
 * Copyright (C) 2018 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

package gov.dot.fhwa.saxton.carma.signal_plugin.ead;

import gov.dot.fhwa.saxton.carma.signal_plugin.dvi.IGlidepathAppConfig;
import gov.dot.fhwa.saxton.carma.signal_plugin.ead.trajectorytree.ICostModel;

/**
 * TODO Factory-pattern class for IAccStrategy objects
 * <p>
 * Used in conjunction with {@link AccStrategyManager} to provide access to ACC functions
 * inside {@link IManeuver} instances.
 */
public class DefaultCostModelFactory implements ICostModelFactory{

  private final IGlidepathAppConfig config;

  public DefaultCostModelFactory(IGlidepathAppConfig config) {
    this.config = config;
  }

  /**
   * Create a new instance of IAccStrategy
   */
  public ICostModel getCostModel(String desiredModelName) {
    switch(desiredModelName) {
      case "DEFAULT":
        double vehicleMass = config.getDoubleValue("ead.vehicleMass");
        double rollingRes = config.getDoubleDefaultValue("ead.rollingResistanceOverride", 0.0);
        double dragCoef = config.getDoubleValue("ead.dragCoefficient");
        double frontalArea = config.getDoubleValue("ead.frontalArea");
        double airDensity = config.getDoubleValue("ead.airDensity");
        double idlePower = config.getDoubleValue("ead.idleCost");
        boolean useIdleMin = config.getBooleanValue("ead.useIdleMin");

        return new FuelCostModel(vehicleMass, rollingRes, dragCoef, frontalArea, airDensity, idlePower, useIdleMin);
      case "MOVES_2010":
        double rollingTermA = config.getDoubleValue("ead.MOVES.rollingTermA");
        double rotatingTermB = config.getDoubleDefaultValue("ead.MOVES.rotatingTermB", 0.0);
        double dragTermC = config.getDoubleValue("ead.MOVES.dragTermC");
        double vehicleMassInTons = config.getDoubleValue("ead.MOVES.vehicleMassInTons");
        double fixedMassFactor = config.getDoubleValue("ead.MOVES.fixedMassFactor");
        String baseRateTablePath = config.getProperty("ead.MOVES.baseRateTablePath");

        return new MovesFuelCostModel(rollingTermA, rotatingTermB, dragTermC, vehicleMassInTons, fixedMassFactor, baseRateTablePath);
      default:
        throw new IllegalArgumentException("Provided cost model name is not valid: " + desiredModelName);
    }
  }
}